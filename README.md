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

## Legal Disclaimer

**IMPORTANT: Read before use.**

This project is provided for **educational and authorized security testing purposes only**. 

### Authorization Requirements
- You MUST have explicit written permission from the network owner before using this tool
- Unauthorized interception of network communications is illegal under federal and state laws
- This tool should ONLY be used on networks you own or have written authorization to test

### Legal Framework
- **Computer Fraud and Abuse Act (CFAA)**: Unauthorized access to computer systems is a federal crime
- **Wiretap Act (18 U.S.C. § 2511)**: Interception of electronic communications without consent is illegal
- **State Laws**: Many states have additional computer crime and wiretapping statutes
- **GDPR/CCPA**: Data collection may be subject to privacy regulations

### Acceptable Use
- Testing security of your own networks
- Authorized penetration testing with written scope
- Academic research in controlled lab environments
- Security education and training

### Prohibited Use
- Intercepting communications on networks you don't own
- Attacking infrastructure without authorization
- Any activity that violates applicable laws or regulations
- Commercial use without proper licensing

### No Warranty
This software is provided "AS IS" without warranty of any kind. The author is not responsible for any misuse or damage caused by this software.

### Responsible Disclosure
If you discover vulnerabilities using this tool, follow responsible disclosure practices:
1. Report to the vendor/owner privately
2. Allow reasonable time for remediation
3. Do not exploit beyond proof of concept

## License

MIT
