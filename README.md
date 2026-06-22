markdown# IoT Posture Detector System

A closed-loop wearable posture monitoring and correction system designed for
long-duration occupational use. The system uses dual IMU sensors to track
spinal alignment in real time, delivers haptic feedback to correct slouching
behaviour, logs session data to an SD card, and streams live metrics to a
web-based dashboard over Bluetooth.

Built and tested as part of an ongoing research effort targeting occupational
driver ergonomics.

---

## Repository Structure
V1/

├── Software/        # Arduino firmware and web dashboard

├── Hardware/        # STL files and wiring schematics

├── Data/            # Session logs and science fair dataset

└── Presentables/    # Judge summary and presentation materials

V2/                  # In development — ESP32, PCB, rechargeable power system

---

## System Overview

Two MPU-6050 IMU sensors are mounted at the upper and lower back. On startup
the device runs a stability-based auto-calibration routine that collects 200
samples, identifies the most stable 50-sample window, removes outliers, and
computes a median baseline — ensuring the reference posture is not corrupted
by movement during initialization.

During monitoring, live sensor readings are continuously compared against the
calibrated baseline. If either sensor detects more than 15° of deviation for
over 2.5 seconds, the vibration motor triggers a haptic correction alert.
Every reading is logged to CSV on an SD card and simultaneously streamed over
Bluetooth to a live web dashboard.

---

## V1 Hardware

| Part | Quantity |
|------|----------|
| Arduino Uno | 1 |
| MPU-6050 IMU sensor | 2 |
| HC-10 Bluetooth module | 1 |
| Micro SD card module | 1 |
| Coin vibration motor | 1 |
| LED | 1 |
| NPN transistor (2N2222) | 1 |
| 1000µF 50V capacitor | 1 |
| 9V battery + holder | 1 |
| Resistors, jumper wires, breadboard | — |

---

## Wiring

| Component | Arduino Pin |
|-----------|------------|
| MPU-6050 upper (0x68) SDA | A4 |
| MPU-6050 upper (0x68) SCL | A5 |
| MPU-6050 lower (0x69) SDA | A4 (shared) |
| MPU-6050 lower (0x69) SCL | A5 (shared) |
| MPU-6050 lower AD0 | 3.3V |
| HC-10 TXD | Pin 8 |
| HC-10 RXD | Pin 9 |
| SD card CS | Pin 5 |
| LED | Pin 7 |
| Motor (via transistor) | Pin 6 |
| Battery positive | VIN |
| Battery negative | GND |
| Capacitor positive | VIN |
| Capacitor negative | GND |

---

## Libraries

- `MPU6050` by Electronic Cats
- `SD` (built in)
- `SoftwareSerial` (built in)
- `Wire` (built in)

---

## Setup

1. Open `V1/Software/Posture_Detector.ino` in Arduino IDE
2. Install required libraries
3. Select **Arduino Uno** under Tools → Board
4. Select correct port under Tools → Port
5. Click Upload

---

## Usage

1. Power on the device
2. Sit or stand in your natural upright posture
3. Hold still — calibration runs automatically and confirms with a buzz
4. Device monitors continuously and vibrates to correct slouching

---

## Dashboard

Open `V1/Software/posture_dashboard.html` in Chrome.

1. Click **Connect to HC-10**
2. Select device from Bluetooth scan list
3. Live pitch, roll, status, and score stream in real time
4. Click **Download CSV** to export session data

---

## Data Format

SD card logs to `posture.csv`:
Time(ms), PitchU, RollU, PitchL, RollL, Status, Score(%)

BLE stream format:
U:0.3/0.0 L:0.2/0.1 GOOD 100%

---

## 3D Printed Enclosure

STL files in `V1/Hardware/`. Printed in PLA on a Bambu Lab P1S.

- Main case: 150mm x 100mm x 40mm
- Sensor case: 40mm x 45mm x 20mm
- Fastened with M3 thumb screws

---

## Known Limitations (V1)

- Bluetooth connection drops after approximately 20–30 minutes due to power
  instability on the HC-10 module
- 9V battery provides inconsistent voltage under motor load
- Single-snapshot calibration vulnerable to initialization error if user
  moves during startup

These are addressed in V2 through an ESP32 migration, LiPo rechargeable
power system, and the robust stability-based calibration algorithm.

---

## Roadmap

| Version | Status | Key upgrades |
|---------|--------|--------------|
| V1 | Complete | Arduino, HC-10, breadboard prototype |
| V2 | In development | ESP32, custom PCB, LiPo power, ML dashboard |

---

## Author

Mehar — Grade 10, Peel Region, Ontario
[GitHub](https://github.com/Mehariscoding)
