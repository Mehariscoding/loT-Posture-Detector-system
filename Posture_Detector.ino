#include <Wire.h>
#include <MPU6050.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

MPU6050 mpuUpper(0x68);
MPU6050 mpuLower(0x69);
SoftwareSerial BTSerial(8, 9);

float baselinePitchU = 0, baselineRollU = 0;
float baselinePitchL = 0, baselineRollL = 0;
const float THRESHOLD = 15.0;
const int LED_PIN = 7;
const int MOTOR_PIN = 6;
const int CS_PIN = 5;

int totalReadings = 0;
int goodReadings = 0;
int badPostureCount = 0;
const int BAD_POSTURE_DELAY = 5;
bool sdAvailable = false;

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);
  Wire.begin();
  pinMode(LED_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);

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

  sdAvailable = SD.begin(CS_PIN);
  if (sdAvailable) {
    Serial.println("SD ready!");
    File dataFile = SD.open("posture.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.println("Time(ms),PitchU,RollU,PitchL,RollL,Status,Score(%)");
      dataFile.close();
    }
  } else {
    Serial.println("SD failed - continuing without it!");
  }

  Serial.println("Stand straight! Auto calibrating in 5 seconds...");
  BTSerial.println("Stand straight! Auto calibrating in 5 seconds...");
  delay(5000);
  calibrate();
}

void calibrate() {
  Serial.println("Calibrating...");
  BTSerial.println("Calibrating...");

  long axSumU=0, aySumU=0, azSumU=0;
  long axSumL=0, aySumL=0, azSumL=0;

  for (int i = 0; i < 100; i++) {
    int16_t ax, ay, az, gx, gy, gz;

    mpuUpper.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    axSumU += ax; aySumU += ay; azSumU += az;

    mpuLower.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    axSumL += ax; aySumL += ay; azSumL += az;

    delay(10);
  }

  baselinePitchU = atan2(aySumU/100.0, azSumU/100.0) * 180.0 / PI;
  baselineRollU  = atan2(axSumU/100.0, azSumU/100.0) * 180.0 / PI;
  baselinePitchL = atan2(aySumL/100.0, azSumL/100.0) * 180.0 / PI;
  baselineRollL  = atan2(axSumL/100.0, azSumL/100.0) * 180.0 / PI;

  Serial.println("Calibrated! Monitoring posture...");
  BTSerial.println("Calibrated! Monitoring posture...");
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(MOTOR_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(MOTOR_PIN, LOW);
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

  BTSerial.print("U:"); BTSerial.print(pitchDiffU, 1);
  BTSerial.print("/"); BTSerial.print(rollDiffU, 1);
  BTSerial.print(" L:"); BTSerial.print(pitchDiffL, 1);
  BTSerial.print("/"); BTSerial.print(rollDiffL, 1);
  BTSerial.print(" "); BTSerial.print(status);
  BTSerial.print(" "); BTSerial.print(score); BTSerial.println("%");

  if (sdAvailable) {
    File dataFile = SD.open("posture.csv", FILE_WRITE);
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