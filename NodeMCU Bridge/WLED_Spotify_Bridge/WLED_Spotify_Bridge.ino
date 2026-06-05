#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// --- CONFIGURATION ---
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* wled_ip = "192.168.X.X"; 
const String vercel_url = "https://YOUR-VERCEL-PROJECT.vercel.app/api/now-playing";
// ---------------------

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    
    WiFiClient client;
    HTTPClient http;
    
    String wledStateUrl = String("http://") + wled_ip + "/json/state";
    http.begin(client, wledStateUrl);
    http.useHTTP10(true); // Prevents ArduinoJson InvalidInput chunking errors
    
    int wledHttpCode = http.GET();
    bool isSpotifyModeActive = false;
    
    if (wledHttpCode == 200) {
      String payload = http.getString();
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        // Reads the true/false status of the "Sync" button in the WLED UI
        bool syncButtonState = doc["udpn"]["send"]; 
        if (syncButtonState == true) {
          isSpotifyModeActive = true;
        }
      }
    }
    http.end();

    if (isSpotifyModeActive) {
      WiFiClientSecure secureClient;
      secureClient.setInsecure(); 
      HTTPClient secureHttp;
      
      secureHttp.begin(secureClient, vercel_url);
      int vercelHttpCode = secureHttp.GET();
      
      if (vercelHttpCode == 200) {
        String vercelPayload = secureHttp.getString();
        JsonDocument vDoc;
        deserializeJson(vDoc, vercelPayload);
        
        bool isPlaying = vDoc["isPlaying"];
        
        if (isPlaying) {
          int r = vDoc["color"][0];
          int g = vDoc["color"][1];
          int b = vDoc["color"][2];
          
          http.begin(client, wledStateUrl);
          http.addHeader("Content-Type", "application/json");
          
          // Pushes the color and ensures the Sync button remains ON
          String colorCmd = "{\"udpn\":{\"send\":true},\"seg\":[{\"col\":[[" + String(r) + "," + String(g) + "," + String(b) + "]]}]}";
          http.POST(colorCmd);
          http.end();
        }
      }
      secureHttp.end();
    }
  }
  delay(3000); // 3-second polling interval to protect Vercel/WLED API limits
}