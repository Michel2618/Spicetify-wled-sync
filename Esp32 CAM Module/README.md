# 📸 ESP32-CAM Ambilight Project (DDP Streamer)

A lightweight, low-latency DIY ambient lighting system that turns any TV into an immersive screen-syncing display using only an ESP32-CAM and WLED.

---

## 🚀 Overview

This project uses an ESP32-CAM module to capture your TV screen in real-time, downsample the image into distinct zones, and broadcast the color data to a WLED-controlled LED strip via the **DDP (Distributed Display Protocol)** over WiFi.

Unlike complex HDMI-capture solutions, this system is:
* **Universal:** Works with built-in TV apps, native channels, and all HDMI inputs.
* **Zero-Hardware:** No expensive HDMI splitters, sync boxes, or capture cards required.
* **Modular:** Operates completely independently of the display device.

---

## 🛠️ Hardware Required

| Component | Description |
| :--- | :--- |
| **Transmitter** | AI-Thinker ESP32-CAM module + Programming Board (e.g., SMARTQAT) |
| **Receiver** | ESP32 DevKit V1 (or similar) running [WLED](https://kno.wled.ge/) |
| **Lighting** | Addressable LED strip (e.g., WS2812B / WS2815) |
| **Power** | 5V Power supply adequate for your total LED strip length |

---

## ⚙️ How it Works

1. **Capture:** The ESP32-CAM captures raw `RGB565` frames at QQVGA (160x120) resolution.
2. **Process:** The firmware samples specific regions along the frame borders (Left, Top, Right) to calculate the dominant color for each physical LED zone.
3. **Transmit:** The calculated RGB values are packed into a 10-byte DDP packet and fired over UDP to the central WLED controller.
4. **Fallback:** WLED's *Realtime Override* feature automatically detects the incoming stream. When the ESP32-CAM is powered off, WLED smoothly reverts to your preset standalone lighting effects.

---

## 💻 Configuration

Before uploading the firmware, update the following constants in the `.ino` file to match your environment:

* `ssid` / `password`: Your home WiFi network credentials.
* `wled_ip`: The static IP address of your WLED controller.
* `TOTAL_LEDS`: The total number of LEDs on your physical strip.
* **LED Mapping:** Adjust the `RIGHT_LEDS`, `TOP_LEDS`, and `LEFT_LEDS` variables to accurately match the physical layout of your strip installation behind the TV.

---

## ⚠️ Known Limitations & Troubleshooting

> **Note on Hardware Stability:** ESP32-CAM modules can be power-hungry and finicky depending on the manufacturer. This firmware accounts for several common hardware quirks.

* **Brownout Prevention:** The code includes a software-based Brownout Detector bypass (`WRITE_PERI_REG`) to handle the sudden power spike during the camera sensor's initialization sequence.
* **Memory Management:** The firmware is explicitly configured to use internal DRAM. This bypasses the unreliable PSRAM often found on clone modules, ensuring stable continuous operation.
* **Camera Timing:** The `xclk_freq_hz` is safely limited to `8000000` (8 MHz) to prevent frame synchronization overflows (`EV-VSYNC-OVF`) on standard ESP32-CAM hardware.
