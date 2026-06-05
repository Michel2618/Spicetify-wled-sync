# 🎵 NodeMCU WLED Spotify Bridge

This folder contains the C++ hardware logic required to bridge the headless Next.js Spotify API with a local WLED network. 

By running this script on a dedicated ESP8266 (NodeMCU V3), the microcontroller acts as a background polling service. It checks the current state of the main WLED light strip and, if authorized via the WLED UI, fetches the actively playing Spotify album art colors to create a dynamic ambient lighting experience.

---

## 🛠️ Hardware Requirements

* **Microcontroller:** NodeMCU V3 (ESP8266)
* **Power:** Standard 5V USB connection (can be plugged into any wall adapter and hidden).
* **Network:** Must be on the same 2.4GHz Wi-Fi network as the target WLED ESP32.

---

## 💻 Software Setup

1. Open the `WLED_Spotify_Bridge/WLED_Spotify_Bridge.ino` file in the **Arduino IDE**.
2. Navigate to **Tools > Board > Boards Manager** and ensure you have the `esp8266` package installed. Select **NodeMCU 1.0 (ESP-12E Module)** as your active board.
3. Navigate to **Sketch > Include Library > Manage Libraries** and install **ArduinoJson** (by Benoit Blanchon).

---

## ⚙️ Configuration

Before flashing the board, you must update the configuration variables at the top of the `.ino` sketch:

```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* wled_ip = "192.168.*.***"; // The static IP of your main WLED strip
const String vercel_url = "[https://your-vercel-project.vercel.app/api/now-playing](https://your-vercel-project.vercel.app/api/now-playing)";
```

## How It Works (The "Sync" Hijack)
To prevent the Spotify API from overriding the lights when you just want to enjoy a static color (like a cohesive pale blue room theme for relaxing or watching movies), this script utilizes a conditional trigger.

Instead of running constantly, the NodeMCU monitors the native **"Sync"** button located at the top left of the WLED mobile/web UI.
* **🟢 Turn ON:** Click the `Sync` button in WLED. The NodeMCU detects the `udpn.send` state change, wakes up, and begins pulling live RGB data from the Vercel API every 3 seconds.
* **🔴 Turn OFF:** Unclick the `Sync` button. The NodeMCU immediately goes to sleep, returning full manual control of the LED strip to you.

## 🎨 Aesthetic Tuning
For the best visual experience, it is highly recommended to increase the WLED transition time:
1. Open the WLED UI.
2. Go to **Config > LED Preferences**.
3. Set the **Transition Time** to `1000ms - 1500ms`. 

