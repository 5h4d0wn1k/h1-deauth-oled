/*
 * H1 — Deauth Detector
 * Real-time WiFi deauthentication detection with OLED display + nRF24L01+ monitoring
 * 
 * Hardware: ESP32 NodeMCU + SSD1306 OLED + nRF24L01+
 * 
 * Wiring:
 *   OLED (software SPI): CLK→D14, DATA→D13, VCC→5V, GND→GND
 *   nRF24 (hardware VSPI): CLK→D18, MOSI→D23, MISO→D19, CSN→D26, CE→D27
 * 
 * Author: 5h4d0wn1k
 * License: MIT
 * Date: 2026-08-26
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <WiFi.h>
#include <esp_wifi.h>

// OLED Configuration (Software SPI)
#define OLED_CLK   14
#define OLED_DATA  13
#define OLED_WIDTH  128
#define OLED_HEIGHT  64
#define OLED_RESET  -1

// nRF24 Configuration (Hardware VSPI)
#define NRF_CSN     26
#define NRF_CE      27
#define NRF_CHANNEL 37  // 2.437 GHz — WiFi ch6

// WiFi Channel
#define WIFI_CHANNEL 6

// Alert Thresholds (deauth frames per second)
#define THRESHOLD_WARNING   5
#define THRESHOLD_CRITICAL  20

// Frame types
#define FRAME_TYPE_DEAUTH      0x00C0
#define FRAME_TYPE_DISASSOC    0x00A0
#define FRAME_TYPE_BEACON      0x0080
#define FRAME_TYPE_PROBE       0x0040

// Global objects
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
RF24 radio(NRF_CE, NRF_CSN);

// Statistics
struct Stats {
    uint32_t deauth_count = 0;
    uint32_t disassoc_count = 0;
    uint32_t beacon_count = 0;
    uint32_t probe_count = 0;
    uint32_t total_frames = 0;
    uint32_t last_deauth_time = 0;
    uint32_t deauth_rate = 0;
    uint8_t threat_level = 0;  // 0=normal, 1=warning, 2=critical
    uint32_t nrf24_packets = 0;
    char last_deauth_mac[18] = "00:00:00:00:00:00";
    uint8_t last_deauth_reason = 0;
};

Stats stats;
uint32_t last_update = 0;
uint32_t last_display_update = 0;
uint32_t deauth_window_start = 0;
uint32_t deauth_window_count = 0;

// Callback for promiscuous mode
void IRAM_ATTR promiscuous_rx(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) return;
    
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    uint16_t frame_ctrl = *(uint16_t*)pkt->payload;
    uint8_t frame_type = (frame_ctrl >> 2) & 0x3;
    uint8_t frame_subtype = (frame_ctrl >> 4) & 0xF;
    
    stats.total_frames++;
    
    // Deauthentication frame
    if (frame_type == 0 && frame_subtype == 12) {
        stats.deauth_count++;
        deauth_window_count++;
        
        // Extract MAC address (sender)
        char mac[18];
        snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
                 pkt->payload[10], pkt->payload[11], pkt->payload[12],
                 pkt->payload[13], pkt->payload[14], pkt->payload[15]);
        strncpy(stats.last_deauth_mac, mac, sizeof(stats.last_deauth_mac));
        
        // Extract reason code
        if (pkt->payload[24] != 0 || pkt->payload[25] != 0) {
            stats.last_deauth_reason = pkt->payload[24];
        }
        
        stats.last_deauth_time = millis();
    }
    // Disassociation frame
    else if (frame_type == 0 && frame_subtype == 10) {
        stats.disassoc_count++;
        deauth_window_count++;
    }
    // Beacon frame
    else if (frame_type == 0 && frame_subtype == 8) {
        stats.beacon_count++;
    }
    // Probe request
    else if (frame_type == 0 && frame_subtype == 4) {
        stats.probe_count++;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== H1 — Deauth Detector ===");
    Serial.println("Initializing...");
    
    // Initialize OLED
    Wire.begin(13, 14);  // SDA, SCL
    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println("OLED initialization failed!");
        while (1) delay(1000);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("H1 Deauth Detector");
    display.println("Initializing...");
    display.display();
    
    // Initialize nRF24L01+
    if (!radio.begin()) {
        Serial.println("nRF24 initialization failed!");
        display.println("nRF24 FAIL");
        display.display();
        while (1) delay(1000);
    }
    
    radio.setChannel(NRF_CHANNEL);
    radio.setPALevel(RF24_PA_LOW);
    radio.setDataRate(RF24_2MBPS);
    radio.startListening();
    
    Serial.println("nRF24 initialized on channel " + String(NRF_CHANNEL));
    
    // Initialize WiFi in promiscuous mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(promiscuous_rx);
    esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
    
    Serial.println("WiFi promiscuous mode active on channel " + String(WIFI_CHANNEL));
    
    // Display ready
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("H1 Deauth Detector");
    display.println("===================");
    display.println();
    display.println("WiFi: Promiscuous");
    display.println("nRF24: Listening");
    display.println("Channel: " + String(WIFI_CHANNEL));
    display.println();
    display.println("Ready!");
    display.display();
    delay(2000);
    
    deauth_window_start = millis();
    last_update = millis();
    last_display_update = millis();
    
    Serial.println("System ready. Monitoring...\n");
}

void loop() {
    uint32_t now = millis();
    
    // Read nRF24 packets (independent observer)
    if (radio.available()) {
        uint8_t data[32];
        radio.read(data, sizeof(data));
        stats.nrf24_packets++;
        
        // Log nRF24 activity (could be MouseJack, ESB, etc.)
        Serial.print("[nRF24] Packet received: ");
        for (int i = 0; i < min((int)sizeof(data), 16); i++) {
            Serial.printf("%02X ", data[i]);
        }
        Serial.println();
    }
    
    // Update deauth rate every second
    if (now - last_update >= 1000) {
        stats.deauth_rate = deauth_window_count;
        deauth_window_count = 0;
        deauth_window_start = now;
        
        // Determine threat level
        if (stats.deauth_rate >= THRESHOLD_CRITICAL) {
            stats.threat_level = 2;  // CRITICAL
        } else if (stats.deauth_rate >= THRESHOLD_WARNING) {
            stats.threat_level = 1;  // WARNING
        } else {
            stats.threat_level = 0;  // NORMAL
        }
        
        last_update = now;
    }
    
    // Update display every 200ms
    if (now - last_display_update >= 200) {
        updateDisplay();
        last_display_update = now;
    }
}

void updateDisplay() {
    display.clearDisplay();
    
    // Header with threat color
    display.setTextSize(1);
    display.setCursor(0, 0);
    
    switch (stats.threat_level) {
        case 0:
            display.println("[NORMAL]  H1 Detector");
            break;
        case 1:
            display.println("[WARNING] H1 Detector");
            break;
        case 2:
            display.println("[CRITICAL] H1 DETECTOR");
            break;
    }
    
    display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
    
    // WiFi statistics
    display.setCursor(0, 14);
    display.printf("Deauth: %lu (%lu/s)", stats.deauth_count, stats.deauth_rate);
    display.setCursor(0, 24);
    display.printf("Disassoc: %lu", stats.disassoc_count);
    display.setCursor(0, 34);
    display.printf("Beacon: %lu", stats.beacon_count);
    display.setCursor(0, 44);
    display.printf("Total: %lu", stats.total_frames);
    
    // nRF24 statistics
    display.setCursor(0, 56);
    display.printf("nRF24: %lu pkts", stats.nrf24_packets);
    
    display.display();
}
