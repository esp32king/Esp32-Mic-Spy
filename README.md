# 🎤 ESP32 Mic Spy I2S Microphone Live Stream (INMP441)

Stream live audio from an **ESP32** with an **INMP441 I2S microphone** to your browser using **WebSocket**! 🌐  
Audio is streamed as **16-bit PCM at 16 kHz, mono**.

> ⚠️ **Warning:** Only use on your local network for authorized testing.

---

## ✨ Features
- Live audio streaming from ESP32 → browser 🎧  
- Works with **I2S microphones** (e.g., INMP441)  
- 16 kHz sample rate, Mono, 16-bit PCM  
- Simple **HTML client** for playback  
- Mobile-friendly and works in modern browsers 📱💻  

---

## 🛠 Hardware Requirements
- ESP32 development board  
- INMP441 I2S Microphone module  
- Jumper wires  

### 🔌 Wiring (ESP32 → INMP441)
| ESP32 Pin | INMP441 Pin | Description       |
|-----------|------------|-----------------|
| 15        | LRCL       | Word Select (WS) |
| 14        | BCLK       | Bit Clock (SCK)  |
| 32        | DOUT       | Data Output      |
| GND       | GND        | Ground           |
| 3.3V/5V   | VDD        | Power            |

---

## 📡 Software Setup (Step by Step)

### 1️⃣ Install Arduino IDE
1. Download from [Arduino IDE](https://www.arduino.cc/en/software)  
2. Install it on your computer  

---

### 2️⃣ Install ESP32 Board Package
1. Open Arduino IDE → **File → Preferences**  
2. In **Additional Boards Manager URLs**, add:

https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

3. Click **OK**  
4. Go to **Tools → Board → Boards Manager**  
5. Search **ESP32** → Click **Install**  

---

### 3️⃣ Select Your ESP32 Board
- Tools → Board → **ESP32 Dev Module**  
- Tools → Port → Select the correct COM port  

---

### 4️⃣ Open the Code
- Copy `ESP32_I2S_WebSocket.ino` into Arduino IDE  

---

### 5️⃣ Update Wi-Fi Credentials
Replace the placeholders in the code with your Wi-Fi network:

`cpp
const char* WIFI_SSID = "Your_SSID";
const char* WIFI_PASS = "Your_PASSWORD";`


---

6️⃣ Upload Code to ESP32

1. Click Upload in Arduino IDE


2. Wait for compilation and flashing to finish


3. Open Serial Monitor (baud rate 115200)


4. Note the ESP32 IP address shown in Serial Monitor




---

🚀 Using the Web Client

1. Open a browser on any device in the same Wi-Fi network


2. Enter the ESP32 IP address, e.g.: http://192.168.1.50/


3. The web page will load a simple client with:

Start button 🎵

Stop button ⏹️

Status display for connection



4. Click Start → Audio streaming begins


5. Click Stop → Streaming stops




---

💻 How It Works

ESP32 reads audio samples from the I2S microphone

Converts 32-bit I2S samples → 16-bit PCM

Streams PCM over WebSocket (/audio)

Browser AudioContext plays the audio in real-time



---

⚙️ Notes & Tips

Works only on local networks (LAN)

Tested with Chrome, Firefox, and Edge

Default buffer size: 1024 samples (can be adjusted in code)

Make sure to use authorized devices only



---

📝 License

MIT License


---

🎨 Screenshots


Simple HTML interface with Start/Stop buttons and live audio playback


---

✅ Your ESP32 I2S live mic streaming setup is ready!
You can now stream audio from your INMP441 to any browser on the same Wi-Fi network.
