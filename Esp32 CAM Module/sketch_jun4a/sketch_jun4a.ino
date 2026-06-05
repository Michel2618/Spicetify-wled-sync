#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>

// --- Network Configuration ---
const char* ssid     = "Michel, I Am Your Router (2.4G)";
const char* password = "Youdontdeserve_2.4";
const char* wled_ip  = "192.168.1.10"; 
const uint16_t wled_port = 4048; // Default WLED DDP port

// --- LED Layout Configuration ---
const int RIGHT_LEDS = 8;
const int TOP_LEDS   = 14;
const int LEFT_LEDS  = 8;
const int TOTAL_LEDS = RIGHT_LEDS + TOP_LEDS + LEFT_LEDS; // 30 LEDs total

WiFiUDP udp;

// Camera Pins for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM    32
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27
#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      21
#define Y4_GPIO_NUM      19
#define Y3_GPIO_NUM      18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

void setup() {
    Serial.begin(115200);
    
    // 1. Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");

    // 2. Initialize Camera for High-Speed RGB Capture
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    
    // We use RGB565 instead of JPEG to skip decoding and increase FPS
    config.pixel_format = PIXFORMAT_RGB565; 
    config.frame_size = FRAMESIZE_QQVGA; // 160x120 resolution
    config.fb_count = 2;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed: 0x%x", err);
        return;
    }
    
    // Adjust camera settings for better screen capture (less glare)
    sensor_t * s = esp_camera_sensor_get();
    s->set_brightness(s, -1); // Darken slightly to focus on screen light
    s->set_saturation(s, 1);  // Boost colors slightly
}

void loop() {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) return;

    // Buffer size = 10 byte DDP header + (30 LEDs * 3 bytes RGB)
    uint8_t packet[10 + (TOTAL_LEDS * 3)];
    
    // Build DDP Network Header
    packet[0] = 0x41; packet[1] = 0x00; packet[2] = 0x01; packet[3] = 0x01;
    packet[4] = 0x00; packet[5] = 0x00;
    packet[6] = (uint8_t)((TOTAL_LEDS * 3) >> 8);
    packet[7] = (uint8_t)((TOTAL_LEDS * 3) & 0xFF);

    uint8_t* ledData = &packet[10];
    int w = fb->width;   // 160
    int h = fb->height;  // 120

    // --- STRIP MAPPING CALCULATION ---
    
    // 1. RIGHT EDGE (LEDs 0 to 7): Moving UP from bottom to top
    for (int i = 0; i < RIGHT_LEDS; i++) {
        int targetX = w - 8; // Sample 8 pixels in from the right edge
        int targetY = h - 1 - ((h / RIGHT_LEDS) * i) - 4;
        if (targetY < 0) targetY = 0;

        int idx = (targetY * w + targetX) * 2;
        uint16_t pixel = (fb->buf[idx] << 8) | fb->buf[idx + 1];
        
        ledData[i * 3]     = ((pixel >> 11) & 0x1F) * 255 / 31; // Red
        ledData[i * 3 + 1] = ((pixel >> 5) & 0x3F) * 255 / 63;  // Green
        ledData[i * 3 + 2] = (pixel & 0x1F) * 255 / 31;         // Blue
    }

    // 2. TOP EDGE (LEDs 8 to 21): Moving RIGHT to LEFT across the top
    for (int i = 0; i < TOP_LEDS; i++) {
        int targetX = w - 1 - ((w / TOP_LEDS) * i) - 4;
        if (targetX < 0) targetX = 0;
        int targetY = 6; // Sample 6 pixels down from the top edge

        int idx = (targetY * w + targetX) * 2;
        uint16_t pixel = (fb->buf[idx] << 8) | fb->buf[idx + 1];
        
        int ledIdx = RIGHT_LEDS + i;
        ledData[ledIdx * 3]     = ((pixel >> 11) & 0x1F) * 255 / 31;
        ledData[ledIdx * 3 + 1] = ((pixel >> 5) & 0x3F) * 255 / 63;
        ledData[ledIdx * 3 + 2] = (pixel & 0x1F) * 255 / 31;
    }

    // 3. LEFT EDGE (LEDs 22 to 29): Moving DOWN from top to bottom
    for (int i = 0; i < LEFT_LEDS; i++) {
        int targetX = 8; // Sample 8 pixels in from the left edge
        int targetY = ((h / LEFT_LEDS) * i) + 4;

        int idx = (targetY * w + targetX) * 2;
        uint16_t pixel = (fb->buf[idx] << 8) | fb->buf[idx + 1];
        
        int ledIdx = RIGHT_LEDS + TOP_LEDS + i;
        ledData[ledIdx * 3]     = ((pixel >> 11) & 0x1F) * 255 / 31;
        ledData[ledIdx * 3 + 1] = ((pixel >> 5) & 0x3F) * 255 / 63;
        ledData[ledIdx * 3 + 2] = (pixel & 0x1F) * 255 / 31;
    }

    // Fire data over UDP to WLED
    udp.beginPacket(wled_ip, wled_port);
    udp.write(packet, sizeof(packet));
    udp.endPacket();

    // Release memory immediately
    esp_camera_fb_return(fb);
    
    // Tiny delay to prevent flooding the router (~50 FPS cap)
    delay(15); 
}