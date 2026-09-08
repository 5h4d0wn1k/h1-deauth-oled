# Deauth Detector Firmware

## Purpose

Real-time WiFi deauthentication detection with OLED display and independent nRF24L01+ 2.4 GHz observer.

## Board

- **Board**: ESP32 NodeMCU + SSD1306 OLED + nRF24L01+
- **FQBN**: `esp32:esp32:esp32`
- **Sketch**: `h1_deauth_oled/h1_deauth_oled.ino`

## Wiring

```
OLED (software SPI): CLK -> D14, DATA -> D13, VCC -> 5V, GND -> GND
nRF24L01+ (hardware VSPI): CLK->D18, MOSI->D23, MISO->D19, CSN->D26, CE->D27, VCC->3V3, GND->GND
```

## Build

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/h1_deauth_oled
# upload (example, ESP32-C6):
# arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyACM0 firmware/h1_deauth_oled
```

## Runtime

See the root README "IMPORTANT" section before powering on. This firmware is
for authorized own-lab study. Serial console exposes the interactive command
set described in the root README. All identifiers in the sketch are
placeholders (`lab-*` SSIDs, `00:11:22:33:44:55`, RFC 5737 / example.com).
