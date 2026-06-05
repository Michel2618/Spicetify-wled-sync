#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include "soc/soc.h"           // Required for brownout detector bypass
#include "soc/rtc_cntl_reg.h"  // Required for brownout detector bypass

// --- Network Configuration ---
const char* ssid     = "*****************"; // Add your WIFI SSID here !
const char* password = "***********"; // Add your WIFI password here !
const char* wled_ip  = "192.168.***.***"; // Add your WLED IP address here !
const uint16_t wled_port = 4048; 

// --- LED Layout Configuration ---
const int RIGHT_LEDS = 8;
const int TOP_LEDS   = 14;
const int LEFT_LEDS  = 8;
const int TOTAL_LEDS = RIGHT_LEDS + TOP_LEDS + LEFT_LEDS; // 30

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
    // 1. DISABLE BROWNOUT DETECTOR
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
    
    Serial.begin(115200);
    Serial.println("\nBooting up... Bypassing PSRAM.");
    
    delay(500); 

    // 2. Connect to WiFi
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    
    delay(500);

    // 3. Initialize Camera for Internal Memory
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
    
    // Slow down the clock so it doesn't overflow
    config.xclk_freq_hz = 8000000;          
    config.pixel_format = PIXFORMAT_RGB565; 
    config.frame_size = FRAMESIZE_QQVGA;    
    config.fb_count = 1;                    
    config.grab_mode = CAMERA_GRAB_LATEST;
    
    // THE FIX: Force the camera to use internal DRAM instead of broken PSRAM
    config.fb_location = CAMERA_FB_IN_DRAM; 

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed: 0x%x", err);
        return;
    }
    
    sensor_t * s = esp_camera_sensor_get();
    s->set_brightness(s, 0); 
    s->set_saturation(s, 2); 
    
    Serial.println("Camera initialized! Starting stream...");
}

void loop() {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Frame capture failed!");
        delay(10);
        return;
    }

    uint8_t packet[10 + (TOTAL_LEDS * 3)];
    
    // Build DDP Header
    packet[0] = 0x41; packet[1] = 0x00; packet[2] = 0x01; packet[3] = 0x01;
    packet[4] = 0x00; packet[5] = 0x00;
    packet[6] = (uint8_t)((TOTAL_LEDS * 3) >> 8);
    packet[7] = (uint8_t)((TOTAL_LEDS * 3) & 0xFF);

    uint8_t* ledData = &packet[10];
    int w = fb->width;   
    int h = fb->height;  

    // 1. RIGHT EDGE (LEDs 0 to 7): Moving UP from bottom to top
    for (int i = 0; i < RIGHT_LEDS; i++) {
        int targetX = w - 8; 
        int targetY = h - 1 - ((h / RIGHT_LEDS) * i) - 4;
        if (targetY < 0) targetY = 0;

        int idx = (targetY * w + targetX) * 2;
        uint16_t pixel = (fb->buf[idx] << 8) | fb->buf[idx + 1];
        
        ledData[i * 3]     = ((pixel >> 11) & 0x1F) * 255 / 31; 
        ledData[i * 3 + 1] = ((pixel >> 5) & 0x3F) * 255 / 63;  
        ledData[i * 3 + 2] = (pixel & 0x1F) * 255 / 31;         
    }

    // 2. TOP EDGE (LEDs 8 to 21): Moving RIGHT to LEFT across the top
    for (int i = 0; i < TOP_LEDS; i++) {
        int targetX = w - 1 - ((w / TOP_LEDS) * i) - 4;
        if (targetX < 0) targetX = 0;
        int targetY = 6; 

        int idx = (targetY * w + targetX) * 2;
        uint16_t pixel = (fb->buf[idx] << 8) | fb->buf[idx + 1];
        
        int ledIdx = RIGHT_LEDS + i;
        ledData[ledIdx * 3]     = ((pixel >> 11) & 0x1F) * 255 / 31;
        ledData[ledIdx * 3 + 1] = ((pixel >> 5) & 0x3F) * 255 / 63;
        ledData[ledIdx * 3 + 2] = (pixel & 0x1F) * 255 / 31;
    }

    // 3. LEFT EDGE (LEDs 22 to 29): Moving DOWN from top to bottom
    for (int i = 0; i < LEFT_LEDS; i++) {
        int targetX = 8; 
        int targetY = ((h / LEFT_LEDS) * i) + 4;

        int idx = (targetY * w + targetX) * 2;
        uint16_t pixel = (fb->buf[idx] << 8) | fb->buf[idx + 1];
        
        int ledIdx = RIGHT_LEDS + TOP_LEDS + i;
        ledData[ledIdx * 3]     = ((pixel >> 11) & 0x1F) * 255 / 31;
        ledData[ledIdx * 3 + 1] = ((pixel >> 5) & 0x3F) * 255 / 63;
        ledData[ledIdx * 3 + 2] = (pixel & 0x1F) * 255 / 31;
    }

    // Send UDP packets to WLED
    udp.beginPacket(wled_ip, wled_port);
    udp.write(packet, sizeof(packet));
    udp.endPacket();

    esp_camera_fb_return(fb);
    delay(20); 
}
