// Created By Krishna Rajput UP61
// Give Me Credit for Use

/* ESP32 I2S (INMP441) -> WebSocket audio stream
   Serve a simple HTML client on "/" and stream binary Int16 PCM over websocket "/audio".
   Sample rate: 16000 Hz, Mono
   NOTE: Use only on your local network for authorized testing.
*/

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "esp_intr_alloc.h"
#include "driver/i2s.h"

const char* WIFI_SSID = "Your_SSID";
const char* WIFI_PASS = "Your_PASSWORD";

#define I2S_WS      15   // LRCL
#define I2S_SD      32   // DOUT (data in)
#define I2S_SCK     14   // BCLK

#define I2S_PORT    I2S_NUM_0
#define SAMPLE_RATE 16000

AsyncWebServer server(80);
AsyncWebSocket ws("/audio");

// Basic HTML client served by ESP32
const char index_html[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8"/>
  <title>ESP32 Live Mic Stream</title>
</head>
<body>
  <h2>ESP32 Live Mic Stream (I2S)</h2>
  <p id="status">Connecting...</p>
  <button id="startBtn">Start</button>
  <button id="stopBtn" disabled>Stop</button>
  <script>
    let socket;
    let audioCtx;
    let scriptNode;
    const sampleRate = 16000;
    const statusEl = document.getElementById('status');
    const startBtn = document.getElementById('startBtn');
    const stopBtn = document.getElementById('stopBtn');

    function setupAudio() {
      audioCtx = new (window.AudioContext || window.webkitAudioContext)({sampleRate: sampleRate});
      // ScriptProcessor (simple demo)
      const bufferSize = 1024;
      scriptNode = audioCtx.createScriptProcessor(bufferSize, 1, 1);
      let queue = [];
      scriptNode.onaudioprocess = function(audioProcessingEvent) {
        const output = audioProcessingEvent.outputBuffer.getChannelData(0);
        if (queue.length === 0) {
          // silence if nothing
          for (let i=0;i<output.length;i++) output[i] = 0;
          return;
        }
        let src = queue.shift();
        // src is Float32Array, maybe length differs
        for (let i=0;i<output.length;i++) {
          output[i] = src[i] || 0;
        }
      };
      scriptNode.connect(audioCtx.destination);
      return {
        push: (float32samples) => queue.push(float32samples)
      };
    }

    let player;
    startBtn.onclick = () => {
      startBtn.disabled = true;
      stopBtn.disabled = false;
      statusEl.textContent = "Connecting websocket...";
      socket = new WebSocket((location.protocol === 'https:' ? 'wss://' : 'ws://') + location.host + "/audio");
      socket.binaryType = "arraybuffer";
      player = setupAudio();
      socket.onopen = () => {
        statusEl.textContent = "Connected, streaming...";
      };
      socket.onmessage = (evt) => {
        // incoming is Int16 PCM (little-endian)
        const ab = evt.data;
        const view = new DataView(ab);
        const len = view.byteLength / 2;
        let float32 = new Float32Array(len);
        for (let i=0;i<len;i++) {
          const int16 = view.getInt16(i*2, true);
          float32[i] = int16 / 32768; // convert to -1..1
        }
        // chunk into blocks matching script node buffer size (1024)
        const chunkSize = 1024;
        for (let i=0;i<float32.length;i+=chunkSize) {
          const sub = float32.subarray(i, i+chunkSize);
          // make copy to detach from underlying buffer (safety)
          player.push(new Float32Array(sub));
        }
      };
      socket.onclose = () => {
        statusEl.textContent = "WebSocket closed";
        startBtn.disabled = false;
        stopBtn.disabled = true;
      };
      socket.onerror = (e) => {
        statusEl.textContent = "WebSocket error";
        console.error(e);
      };
    };

    stopBtn.onclick = () => {
      if (socket) socket.close();
      startBtn.disabled = false;
      stopBtn.disabled = true;
      statusEl.textContent = "Stopped";
    };
  </script>
</body>
</html>
)rawliteral";


void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type,
               void * arg, uint8_t *data, size_t len){
  // No special handling required for this demo
  if(type == WS_EVT_CONNECT){
    Serial.printf("WebSocket client #%u connected\n", client->id());
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
  }
}

void i2s_init() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 0, // default interrupt allocation
    .dma_buf_count = 6,
    .dma_buf_len = 512,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };
  i2s_set_pin(I2S_PORT, &pin_config);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP: ");
  Serial.println(WiFi.localIP());

  // I2S init
  i2s_init();

  // Web server + websocket
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.begin();
  Serial.println("Server started");
}

void loop() {
  // If no clients, skip reading to save CPU
  if (ws.count() == 0) {
    delay(50);
    return;
  }

  // Read samples from I2S. Using 32-bit read buffer because many I2S mics output 24-bit in 32-bit container.
  const int READ_SAMPLES = 1024; // number of 32-bit words
  static int32_t i2s_buffer[READ_SAMPLES];
  size_t bytes_read = 0;
  esp_err_t r = i2s_read(I2S_PORT, (void*)i2s_buffer, READ_SAMPLES * sizeof(int32_t), &bytes_read, portMAX_DELAY);
  if (r != ESP_OK || bytes_read == 0) {
    // nothing read
    return;
  }

  // bytes_read is in bytes. Convert 32-bit samples to int16_t PCM (little endian)
  size_t samples_read = bytes_read / sizeof(int32_t);
  // prepare a temporary Int16 buffer
  static int16_t outbuf[READ_SAMPLES];
  for (size_t i = 0; i < samples_read; ++i) {
    // i2s_buffer[i] typically has the 24-bit sample in MSB of 32-bit container.
    // Convert: shift right by 16 to get top 16 bits (simple conversion)
    int32_t s32 = i2s_buffer[i];
    int16_t s16 = (int16_t)(s32 >> 16); // crude but works for demo
    outbuf[i] = s16;
  }

  // Send raw int16 PCM bytes over websocket to all clients
  // Note: bytes to send = samples_read * 2
  ws.binaryAll((uint8_t*)outbuf, samples_read * sizeof(int16_t));
}
