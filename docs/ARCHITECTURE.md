# H1 — Deauth Detector Architecture

## System Overview

The H1 Deauth Detector is a standalone WiFi intrusion detection system that combines two independent monitoring channels:

1. **WiFi Promiscuous Mode** — Direct frame analysis for deauth/disassoc detection
2. **nRF24L01+ Radio** — Independent 2.4 GHz observer (outside WiFi stack)

This dual-channel approach provides:
- Redundancy: If WiFi stack is jammed, nRF24 still sees activity
- Independence: nRF24 operates outside the ESP32's WiFi stack
- Extended coverage: nRF24 can detect non-WiFi protocols (MouseJack, ESB)

## Architecture Diagram

```
                    ┌─────────────────┐
                    │   WiFi Traffic   │
                    │  (2.4 GHz)       │
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │  ESP32 NodeMCU  │
                    │  (Promiscuous)  │
                    │                 │
                    │  ┌────────────┐ │
                    │  │ Frame Parser│ │
                    │  │  (Deauth)  │ │
                    │  └─────┬──────┘ │
                    │        │        │
                    │  ┌─────▼──────┐ │
                    │  │  Stats     │ │
                    │  │  Engine    │ │
                    │  └─────┬──────┘ │
                    └────────┼────────┘
                             │
              ┌──────────────┼──────────────┐
              │              │              │
     ┌────────▼────────┐    │    ┌─────────▼────────┐
     │  OLED Display   │    │    │  nRF24L01+       │
     │  (Real-time)    │    │    │  (Independent)   │
     └─────────────────┘    │    └──────────────────┘
                            │
                    ┌───────▼────────┐
                    │  Serial Log    │
                    │  (Forensics)   │
                    └────────────────┘
```

## Frame Processing

### WiFi Promiscuous Mode

The ESP32 operates in promiscuous mode, capturing all WiFi management frames:

1. **Beacon Frames** (Type 0, Subtype 8): SSID discovery, AP enumeration
2. **Probe Requests** (Type 0, Subtype 4): Client discovery
3. **Deauthentication** (Type 0, Subtype 12): **Primary target**
4. **Disassociation** (Type 0, Subtype 10): Secondary target

### Deauth Frame Structure

```
Bytes 0-1:   Frame Control (0x00C0 for deauth)
Bytes 2-3:   Duration
Bytes 4-9:   Destination MAC
Bytes 10-15: Source MAC (attacker)
Bytes 16-21: BSSID (target AP)
Bytes 22-23: Sequence Number
Bytes 24-25: Reason Code
```

### Common Reason Codes

| Code | Meaning | Attack Context |
|------|---------|----------------|
| 1 | Unspecified | Generic deauth |
| 2 | Auth no longer valid | Session hijack |
| 3 | Deauth: STA leaving | Forced disconnect |
| 4 | Disassoc: Inactivity | Sleep deprivation |
| 5 | Disassoc: AP unable | Resource exhaustion |
| 7 | Class 3 frame from non-associated | Rogue deauth |

## nRF24L01+ Monitoring

The nRF24L01+ operates on channel 37 (2.437 GHz = WiFi channel 6) as an independent observer:

### What nRF24 Detects

- MouseJack attacks (wireless keyboard/mouse injection)
- ESP-NOW packets (ESP32 mesh traffic)
- NRF24L01+ proprietary protocols
- General 2.4 GHz activity

### Why Independent?

The nRF24 operates outside the ESP32's WiFi stack, providing:
- Redundancy if WiFi is jammed
- Detection of non-WiFi protocols
- Separate radio chain (no shared resources)

## Threat Assessment

### Rate-Based Detection

The system calculates deauth rate over 1-second windows:

```
deauth_rate = count(deauth_frames) in last_1000ms
```

### Threat Levels

| Level | Condition | Response |
|-------|-----------|----------|
| NORMAL | rate < 5 | Display updates, serial log |
| WARNING | 5 ≤ rate < 20 | OLED warning, increased logging |
| CRITICAL | rate ≥ 20 | OLED critical alert, MAC logging |

### False Positive Mitigation

1. **Rate threshold**: Normal background noise rarely exceeds 2 deauth/s
2. **MAC tracking**: Identify persistent attackers vs random noise
3. **Reason code analysis**: Certain reason codes indicate attacks vs legitimate disconnects

## Serial Forensics Log

All detected attacks are logged with:
- Timestamp (millis)
- Frame type (deauth/disassoc)
- Attacker MAC address
- Reason code
- nRF24 activity

This data feeds into:
- **D4 — Network Forensics Suite**: PCAP-style analysis
- **X1 — Adversarial YAML**: MITRE ATT&CK mapping

## Research Applications

### W1 — Deauth Engine + IDS

H1 pairs with H7 (Evil Twin AP) on C6 to create a complete attack/defense pair:
- C6: Attacker (deauth + evil twin)
- NodeMCU: Defender (detection + alerting)
- nRF24: Independent observer (out-of-band)

### P1 — Adversarial CSI

Understanding deauth frame patterns enables:
- Timing analysis for motion detector evasion
- Frame injection strategies for CSI manipulation

### P2 — ESP-NOW Security SoK

nRF24 monitoring of ESP-NOW traffic reveals:
- Mesh network vulnerabilities
- Authentication weaknesses
- Replay attack surfaces

## Future Enhancements

1. **SD card logging**: Store all frames for offline analysis
2. **Bluetooth LE monitoring**: Add BLE sniffer capability
3. **Machine learning**: Anomaly detection for adaptive thresholds
4. **Network integration**: Forward alerts to SIEM (X5)

## References

- IEEE 802.11 Management Frames
- ESP32 WiFi Promiscuous Mode Documentation
- nRF24L01+ Datasheet
- WiFi Deauthentication Attacks (Research)
