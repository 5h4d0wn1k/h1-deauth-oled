# H1 — Deauth Detector

Real-time WiFi deauthentication detection with OLED display and nRF24L01+ monitoring.

## Overview

This project implements a standalone WiFi deauthentication detector that:
- Monitors WiFi traffic in promiscuous mode for deauth/disassoc frames
- Displays real-time statistics on SSD1306 OLED
- Uses nRF24L01+ as an independent RF observer (separate from WiFi stack)
- Logs all detected attacks with MAC addresses and reason codes
- Escalates alerts based on deauth rate (normal → warning → critical)

## Hardware

| Component | Connection | Role |
|-----------|------------|------|
| ESP32 NodeMCU | Main board | WiFi monitoring, OLED display |
| SSD1306 OLED | Software SPI (D13/D14) | Real-time status display |
| nRF24L01+ | Hardware VSPI (D18/D23) | Independent 2.4 GHz observer |

## Wiring

```
OLED (Software SPI):
  CLK  → D14 (GPIO14)
  DATA → D13 (GPIO13)
  VCC  → 5V rail
  GND  → GND rail

nRF24L01+ (Hardware VSPI):
  CLK  → D18 (GPIO18)
  MOSI → D23 (GPIO23)
  MISO → D19 (GPIO19)
  CSN  → D26 (GPIO26)
  CE   → D27 (GPIO27)
  VCC  → 3.3V
  GND  → GND rail
```

## Alert Levels

| Level | Threshold | Meaning |
|-------|-----------|---------|
| NORMAL | < 5 deauth/s | Background noise |
| WARNING | 5-19 deauth/s | Possible attack |
| CRITICAL | ≥ 20 deauth/s | Active deauth attack |

## Serial Output

```
=== H1 — Deauth Detector ===
WiFi promiscuous mode active on channel 6
nRF24 initialized on channel 37

[DEAUTH] 00:11:22:33:44:55 reason=7
[DEAUTH] AA:BB:CC:DD:EE:FF reason=3
[nRF24] Packet received: 01 02 03 ...
```

## Build & Flash

```bash
# Using Arduino CLI
arduino-cli compile --fqbn esp32:esp32:esp32 h1_deauth_oled
arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 h1_deauth_oled
```

## Research Value

This detector feeds into:
- **W1 — Deauth Engine + IDS**: Pair with C6 attacker for full attack/defense research
- **P1 — Adversarial CSI**: Understand deauth frame patterns for motion detector evasion
- **P2 — ESP-NOW Security SoK**: Map the attack surface of WiFi management frames

## License

MIT
