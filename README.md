<img width="1408" height="768" alt="Cover Image for the Project AmSpice" src="https://github.com/user-attachments/assets/e1a1eecb-5afd-45a9-8ad9-72adfab3c6c4" />

# 🌌 Ambient Light Ecosystem

A comprehensive, modular DIY ambient lighting suite designed to transform any environment using ESP32-based hardware and cloud-integrated software.

[![WLED](https://img.shields.io/badge/Powered_by-WLED-blue.svg)](https://kno.wled.ge/)
---

## 📖 System Overview

This repository contains an interconnected set of tools that provide two distinct ambient lighting experiences:

* **🎵 Cloud-Synced Audio Reactive Lighting:** Pulls live Spotify data to visualize music colors based on album art.
* **📺 Real-Time Screen Mirroring:** Uses local computer vision to project TV/monitor screen colors onto LED strips.

---

## ⚙️ Core Components

### 1. WLED Cloud Sync API
*The "brain" for cloud-based music synchronization.*
* **Tech Stack:** Next.js, Vercel, `fast-average-color-node`
* **Key Feature:** Implements a custom vibrancy algorithm that filters out dark shadows and forces color saturation. This ensures that even pale or muted album art translates into vivid, punchy LED colors.

### 2. NodeMCU Bridge (Spotify Sync)
*The bridge between your cloud data and local hardware.*
* **Tech Stack:** C++, ESP8266, ArduinoJson
* **Key Feature:** Features **"Sync Hijack"** logic. The bridge continuously monitors the WLED native *Sync* button state, allowing you to seamlessly toggle Spotify-synced mode on or off instantly without needing external apps.

### 3. ESP32-CAM Module (DDP Streamer)
*Real-time screen-syncing for displays.*
* **Tech Stack:** C++, ESP32-CAM, Distributed Display Protocol (DDP)
* **Key Feature:** Captures and downsamples TV screen frames using the onboard camera. It calculates dominant zone colors on the edge and streams them via UDP directly to your WLED controller.

---

## 🕸️ Interconnectivity

The ecosystem is unified by the **WLED Controller (ESP32 DevKit V1)**, which acts as the central display engine. 

It seamlessly manages the **"Realtime Override"** state, automatically switching between the Cloud-Sync API (for music) and the ESP32-CAM Streamer (for video) based purely on which module is currently transmitting data packets.

---

## 🚀 Quick Start Configuration

To get the system running, ensure all modules are properly configured to communicate with your central WLED IP address.

### Step 1: API Configuration
1. Navigate to the API source directory.
2. Create or update your `.env.local` file.
3. Add your Spotify OAuth credentials to authenticate the live data stream.

### Step 2: Hardware Flashing (Bridge & Streamer)
1. Open the respective `.ino` files for the Bridge and Streamer.
2. Update the following constants with your local network details:
   * `ssid`
   * `password`
   * `wled_ip` (The static IP of your central ESP32 DevKit V1)
3. Flash the code to your ESP8266 and ESP32-CAM modules.

---

*Developed as a custom IoT ambient light solution.*
