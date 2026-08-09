
# Changelog

## V2 — In Development

### Hardware
- Replaced Arduino Uno + HC-05 with ESP32 Huzzah 32
- Removed HC-10 Bluetooth module — replaced by ESP32 native BLE
- Replaced 9V battery with LiPo 3000mAh + TP4056 charging circuit
- Added MT3608 boost converter for stable 5V output
- Added slide switch for clean power on/off
- Custom PCB in development (replacing breadboard)
- Redesigned wearable enclosure — dual elastic band system with TPU cable sleeve

### Firmware
- Rewrote calibration system — stability-based window selection, outlier rejection, median baseline
- Migrated Bluetooth from SoftwareSerial HC-10 to ESP32 native BLE (NUS protocol)
- Confirmed 122-minute continuous BLE connection with zero dropouts (V1 dropped at ~20-30 min)

### Dashboard
- Full rewrite for Web Bluetooth API
- Added real-time ML inference in browser
- Multi-class posture classifier (Nearest-Centroid, 84.6% accuracy, trained on 15,457 samples)
- Predictive deterioration model (linear regression, 30-second lookahead)
- Anomaly detection (IQR method, asymmetric loading detection)
- Fatigue index (analytics)
- Session intelligence report (analytics)

### Data
- 13,874 readings over 122 minutes — zero Bluetooth dropouts confirmed

---

## V1 — Complete
Built for Peel Region Science Fair 2025.

### Hardware
- Arduino Uno
- HC-10 Bluetooth module
- Dual MPU-6050 IMU sensors
- SD card logging
- 9V battery
- Coin vibration motor + LED alerts
- 3D printed enclosure (PLA)

### Known Limitations
- Bluetooth dropout at ~20-30 minutes due to power instability
- Single-snapshot calibration vulnerable to initialization error
- Non-rechargeable power system
