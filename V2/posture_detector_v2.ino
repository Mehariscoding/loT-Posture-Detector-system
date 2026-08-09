#include <Wire.h>
#include <MPU6050.h>
#include <SPI.h>
#include <SD.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

MPU6050 mpuUpper(0x68);
MPU6050 mpuLower(0x69);

// BLE setup
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) { deviceConnected = true; }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    BLEDevice::startAdvertising();
  }
};

float baselinePitchU = 0, baselineRollU = 0;
float baselinePitchL = 0, baselineRollL = 0;
const float THRESHOLD = 15.0;
const int LED_PIN = 12;
const int MOTOR_PIN = 14;
const int CS_PIN = 21;

int totalReadings = 0;
int goodReadings = 0;
int badPostureCount = 0;
const int BAD_POSTURE_DELAY = 5;
bool sdAvailable = false;

// Calibration constants
const int CAL_SAMPLES = 200;
const int WINDOW_SIZE = 50;
const float STABILITY_THRESHOLD = 2.0;

struct SensorReading {
  float pitchU, rollU, pitchL, rollL;
};

float computeVariance(float* arr, int n) {
  float sum = 0;
  for (int i = 0; i < n; i++) sum += arr[i];
  float mean = sum / n;
  float var = 0;
  for (int i = 0; i < n; i++) var += (arr[i] - mean) * (arr[i] - mean);
  return var / n;
}

float computeMedian(float* arr, int n) {
  float sorted[n];
  for (int i = 0; i < n; i++) sorted[i] = arr[i];
  for (int i = 1; i < n; i++) {
    float key = sorted[i];
    int j = i - 1;
    while (j >= 0 && sorted[j] > key) {
      sorted[j + 1] = sorted[j];
      j--;
    }
    sorted[j + 1] = key;
  }
  return (n % 2 == 0) ? (sorted[n/2 - 1] + sorted[n/2]) / 2.0 : sorted[n/2];
}

void filterOutliers(float* arr, int n) {
  float sum = 0;
  for (int i = 0; i < n; i++) sum += arr[i];
  float mean = sum / n;
  float var = 0;
  for (int i = 0; i < n; i++) var += (arr[i] - mean) * (arr[i] - mean);
  float stddev = sqrt(var / n);
  for (int i = 0; i < n; i++) {
    if (abs(arr[i] - mean) > 2.0 * stddev) arr[i] = mean;
  }
}

void sendBLE(String message) {
  if (deviceConnected) {
    pCharacteristic->setValue(message.c_str());
    pCharacteristic->notify();
  }
}

void calibrate() {
  bool calibrated = false;

  while (!calibrated) {
    Serial.println("Calibrating - hold still...");
    sendBLE("Calibrating - hold still...");

    SensorReading buffer[CAL_SAMPLES];

    for (int i = 0; i < CAL_SAMPLES; i++) {
      int16_t ax, ay, az, gx, gy, gz;

      mpuUpper.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
      buffer[i].pitchU = atan2((float)ay, (float)az) * 180.0 / PI;
      buffer[i].rollU  = atan2((float)ax, (float)az) * 180.0 / PI;

      mpuLower.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
      buffer[i].pitchL = atan2((float)ay, (float)az) * 180.0 / PI;
      buffer[i].rollL  = atan2((float)ax, (float)az) * 180.0 / PI;

      delay(10);
    }

    int bestWindow = 0;
    float bestScore = 999999;

    for (int w = 0; w <= CAL_SAMPLES - WINDOW_SIZE; w++) {
      float pu[WINDOW_SIZE], ru[WINDOW_SIZE];
      float pl[WINDOW_SIZE], rl[WINDOW_SIZE];

      for (int i = 0; i < WINDOW_SIZE; i++) {
        pu[i] = buffer[w + i].pitchU;
        ru[i] = buffer[w + i].rollU;
        pl[i] = buffer[w + i].pitchL;
        rl[i] = buffer[w + i].rollL;
      }

      float score = computeVariance(pu, WINDOW_SIZE)
                  + computeVariance(ru, WINDOW_SIZE)
                  + computeVariance(pl, WINDOW_SIZE)
                  + computeVariance(rl, WINDOW_SIZE);

      if (score < bestScore) {
        bestScore = score;
        bestWindow = w;
      }
    }

    if (bestScore > STABILITY_THRESHOLD) {
      Serial.println("Too much movement - recalibrating...");
      sendBLE("Too much movement - recalibrating...");
      delay(2000);
      continue;
    }

    float pu[WINDOW_SIZE], ru[WINDOW_SIZE];
    float pl[WINDOW_SIZE], rl[WINDOW_SIZE];

    for (int i = 0; i < WINDOW_SIZE; i++) {
      pu[i] = buffer[bestWindow + i].pitchU;
      ru[i] = buffer[bestWindow + i].rollU;
      pl[i] = buffer[bestWindow + i].pitchL;
      rl[i] = buffer[bestWindow + i].rollL;
    }

    filterOutliers(pu, WINDOW_SIZE);
    filterOutliers(ru, WINDOW_SIZE);
    filterOutliers(pl, WINDOW_SIZE);
    filterOutliers(rl, WINDOW_SIZE);

    baselinePitchU = computeMedian(pu, WINDOW_SIZE);
    baselineRollU  = computeMedian(ru, WINDOW_SIZE);
    baselinePitchL = computeMedian(pl, WINDOW_SIZE);
    baselineRollL  = computeMedian(rl, WINDOW_SIZE);

    calibrated = true;

    float confidence = 1.0 / (1.0 + bestScore);
    Serial.print("Calibrated! Confidence: ");
    Serial.println(confidence, 4);
    sendBLE("Calibrated! Confidence: " + String(confidence, 4));

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(MOTOR_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(MOTOR_PIN, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(23, 22);
  pinMode(LED_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);

  // BLE init
  BLEDevice::init("PostureDetector_V2");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  BLEDevice::startAdvertising();
  Serial.println("BLE started - PostureDetector_V2");

  mpuUpper.initialize();
  mpuLower.initialize();

  if (!mpuUpper.testConnection()) {
    Serial.println("Upper MPU failed!");
    while (1);
  }
  if (!mpuLower.testConnection()) {
    Serial.println("Lower MPU failed!");
    while (1);
  }

  SPI.begin(5, 19, 18, 21);
  sdAvailable = SD.begin(CS_PIN);
  if (sdAvailable) {
    Serial.println("SD ready!");
    File dataFile = SD.open("/posture.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.println("Time(ms),PitchU,RollU,PitchL,RollL,Status,Score(%)");
      dataFile.close();
    }
  } else {
    Serial.println("SD failed - continuing without it!");
  }

  Serial.println("Stand straight! Auto calibrating in 5 seconds...");
  sendBLE("Stand straight! Auto calibrating in 5 seconds...");
  delay(5000);
  calibrate();
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;

  mpuUpper.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  float pitchU = atan2((float)ay, (float)az) * 180.0 / PI;
  float rollU  = atan2((float)ax, (float)az) * 180.0 / PI;

  mpuLower.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  float pitchL = atan2((float)ay, (float)az) * 180.0 / PI;
  float rollL  = atan2((float)ax, (float)az) * 180.0 / PI;

  float pitchDiffU = abs(pitchU - baselinePitchU);
  float rollDiffU  = abs(rollU  - baselineRollU);
  float pitchDiffL = abs(pitchL - baselinePitchL);
  float rollDiffL  = abs(rollL  - baselineRollL);

  totalReadings++;
  String status;

  if (pitchDiffU > THRESHOLD || rollDiffU > THRESHOLD ||
      pitchDiffL > THRESHOLD || rollDiffL > THRESHOLD) {
    badPostureCount++;
    if (badPostureCount >= BAD_POSTURE_DELAY) {
      status = "BAD";
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(MOTOR_PIN, HIGH);
    } else {
      status = "ADJUSTING";
      digitalWrite(LED_PIN, LOW);
      digitalWrite(MOTOR_PIN, LOW);
    }
  } else {
    badPostureCount = 0;
    goodReadings++;
    status = "GOOD";
    digitalWrite(LED_PIN, LOW);
    digitalWrite(MOTOR_PIN, LOW);
  }

  int score = (goodReadings * 100) / totalReadings;

  Serial.print("U-Pitch: "); Serial.print(pitchDiffU, 1);
  Serial.print(" U-Roll: "); Serial.print(rollDiffU, 1);
  Serial.print(" | L-Pitch: "); Serial.print(pitchDiffL, 1);
  Serial.print(" L-Roll: "); Serial.print(rollDiffL, 1);
  Serial.print(" | "); Serial.print(status);
  Serial.print(" | Score: "); Serial.print(score); Serial.println("%");

  String bleMsg = "U:" + String(pitchDiffU, 1) + "/" + String(rollDiffU, 1)
                + " L:" + String(pitchDiffL, 1) + "/" + String(rollDiffL, 1)
                + " " + status + " " + String(score) + "%";
  sendBLE(bleMsg);

  if (sdAvailable) {
    File dataFile = SD.open("/posture.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.print(millis()); dataFile.print(",");
      dataFile.print(pitchDiffU, 1); dataFile.print(",");
      dataFile.print(rollDiffU, 1); dataFile.print(",");
      dataFile.print(pitchDiffL, 1); dataFile.print(",");
      dataFile.print(rollDiffL, 1); dataFile.print(",");
      dataFile.print(status); dataFile.print(",");
      dataFile.println(score);
      dataFile.close();
    }
  }

  delay(500);
}