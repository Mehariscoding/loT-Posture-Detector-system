# loT-Posture-Detector-system
IoT wearable that tracks spinal alignment using two gyroscope sensors, logs data to an SD card, and streams live posture data over Bluetooth to a web dashboard

# IoT Posture Detector System

A wearable Arduino-based device that monitors upper and lower back alignment in real time using dual MPU-6050 IMU sensors. When slouching is detected, the device alerts the user via vibration and LED. All session data is logged to an SD card and streamed live over Bluetooth to a web dashboard.

Built for the Peel Region Science Fair 2025.

---

## How it works

Two MPU-6050 gyroscope/accelerometer sensors are mounted on the upper and lower back. On startup the device auto-calibrates to the user's ideal posture over 5 seconds. From that point it continuously compares live sensor readings against the baseline — if either sensor detects more than 15° of deviation for over 2.5 seconds, it triggers a vibration motor and LED alert.

Every reading is logged to a CSV file on the SD card and simultaneously streamed over Bluetooth to a live web dashboard built in HTML/JS.

---

## Components

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

## Libraries required

Install these in Arduino IDE before uploading:

- `MPU6050` by Electronic Cats
- `SD` (built in)
- `SoftwareSerial` (built in)
- `Wire` (built in)

---

## How to upload

1. Open `posture_detector.ino` in Arduino IDE
2. Install the required libraries above
3. Select **Arduino Uno** under Tools → Board
4. Select the correct port under Tools → Port
5. Click Upload

---

## How to use

1. Power on the device
2. Stand or sit in your ideal posture
3. Wait 5 seconds for auto-calibration — LED and motor will buzz once to confirm
4. The device now monitors your posture continuously
5. If you slouch for more than 2.5 seconds the LED lights up and motor vibrates

---

## Web dashboard

Open `posture_dashboard.html` in **Chrome** on a Mac or PC.

1. Click **Connect to HC-10**
2. Select your device from the Bluetooth scan list
3. Live posture data streams in real time — pitch, roll, status, score
4. Click **Download CSV** at any time to save your session data

---

## Data format

The SD card logs to `posture.csv` with the following columns:
```
Time(ms), PitchU, RollU, PitchL, RollL, Status, Score(%)
```

The BLE stream format is:
```
U:0.3/0.0 L:0.2/0.1 GOOD 100%
```

---

## 3D printed case

The `case/` folder contains STL files for the main electronics enclosure and the lower sensor housing. Printed in PLA on a Bambu Lab P1S.

- Main case: 150mm x 100mm x 40mm
- Sensor case: 40mm x 45mm x 20mm
- Fastened with M3 thumb screws

---

## Author

Mehar — Grade 10, Peel Region, Ontario  
[GitHub](https://github.com/Mehariscoding)
