# Spicetify WLED Sync 🎵💡

A lightweight extension for [Spicetify](https://spicetify.app/) that automatically syncs your WLED addressable LED strips with the dominant color of the currently playing Spotify album art.

## Features
* **Zero API Keys:** Uses local network communication (HTTP Request) to talk directly to your WLED controller.
* **Event Driven:** Updates instantly when you change songs (PC or Phone via Spotify Connect).
* **Smart Color Boosting:** Automatically brightens dark album covers so your lights don't turn off or look muddy.
* **White Balance Fix:** Detects washed-out "white" album covers and extracts the vibrant accent color instead.

## Prerequisites
1.  **Spicetify:** Installed on your Spotify client.
2.  **WLED Device:** An ESP32/ESP8266 running WLED, connected to the same Wi-Fi network as your PC.

## Installation

### 1. Download the Script
Download the `wled.js` file from this repository.

### 2. Add to Spicetify
Move the file into your Spicetify Extensions folder:
* **Windows:** `%appdata%\spicetify\Extensions\`
* **Linux/Mac:** `~/.config/spicetify/Extensions/`

### 3. Configure Your IP
Open `wled.js` in a text editor (Notepad, VS Code) and find the top line:

```javascript
const WLED_IP = "YOUR_WLED_IP_HERE"; // Change this to your WLED IP (e.g., "192.168.1.15")
