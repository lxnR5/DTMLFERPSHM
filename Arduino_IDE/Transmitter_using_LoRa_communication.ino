#include <Wire.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <DHT.h>
#include <SPI.h>
#include <LoRa.h>
#include "MAX30105.h"
#include "heartRate.h"

#define MPU_ADDR 0x68

// ================= GPS =================
HardwareSerial GPSSerial(1);
TinyGPSPlus gps;

// ================= DHT22 =================
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ================= LoRa =================
#define SS   5
#define RST  14
#define DIO0 26

// ================= MAX30102 =================
MAX30105 particleSensor;

const byte RATE_SIZE = 8;
byte rates[RATE_SIZE];
byte rateSpot = 0;

long lastBeat = 0;

float beatsPerMinute = 0;
int beatAvg = 0;

float spo2 = 0;
float filteredSpO2 = 0;

#define SPO2_SAMPLES 50
float redBuffer[SPO2_SAMPLES];
float irBuffer[SPO2_SAMPLES];
int bufferIndex = 0;

// ================= VARIABLES =================
unsigned long lastPrint = 0;
unsigned long lastDHTRead = 0;

float temperature = 0;
float humidity = 0;

float latitude = 0;
float longitude = 0;

int16_t ax = 0;
int16_t ay = 0;
int16_t az = 0;

// ===== Communication Metrics =====
unsigned long packetID = 0;
unsigned long txTime = 0;
unsigned long ackStart = 0;
unsigned long rtt = 0;
float latency = 0;
// ================= MPU FUNCTIONS =================
void writeRegister(uint8_t reg, uint8_t data) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

int16_t read16(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2);

  return Wire.read() << 8 | Wire.read();
}

// ================= SETUP =================
void setup() {

  Serial.begin(115200);
  delay(2000);

  Wire.begin(21, 22);

  // MPU6050 Wakeup
  writeRegister(0x6B, 0x00);

  // GPS
  GPSSerial.begin(9600, SERIAL_8N1, 16, 17);

  // DHT22
  dht.begin();

  // MAX30102
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found!");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeGreen(0);

  // LoRa
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa Failed!");
    while (1);
  }

  Serial.println("LoRa Transmitter Ready!");

  // CSV Header
  Serial.println("BPM,SpO2,Temp,Humidity,Motion(AX),Motion(AY),Motion(AZ),Lat,Lon");
}

// ================= LOOP =================
void loop() {

  // ---------- GPS ----------
  while (GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }

  if (gps.location.isUpdated()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
  }

// ---------- MAX30102 ----------
long irValue = particleSensor.getIR();
long redValue = particleSensor.getRed();

// Finger detection
if (irValue < 50000) {

  beatAvg = 0;
  filteredSpO2 = 0;

} else {

  // BPM Detection
  if (checkForBeat(irValue)) {

    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute > 50 && beatsPerMinute < 120) {

      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      beatAvg = 0;

      for (byte x = 0; x < RATE_SIZE; x++) {
        beatAvg += rates[x];
      }

      beatAvg /= RATE_SIZE;
    }
  }

  // ---------- SpO2 ----------
  redBuffer[bufferIndex] = redValue;
  irBuffer[bufferIndex] = irValue;

  bufferIndex++;

  if (bufferIndex >= SPO2_SAMPLES) {

    bufferIndex = 0;

    float redAC = 0;
    float redDC = 0;

    float irAC = 0;
    float irDC = 0;

    for (int i = 0; i < SPO2_SAMPLES; i++) {
      redDC += redBuffer[i];
      irDC += irBuffer[i];
    }

    redDC /= SPO2_SAMPLES;
    irDC /= SPO2_SAMPLES;

    for (int i = 0; i < SPO2_SAMPLES; i++) {
      redAC += abs(redBuffer[i] - redDC);
      irAC += abs(irBuffer[i] - irDC);
    }

    redAC /= SPO2_SAMPLES;
    irAC /= SPO2_SAMPLES;

    float R = (redAC / redDC) / (irAC / irDC);

    spo2 = 110 - (25 * R);

    filteredSpO2 = (0.7 * filteredSpO2) + (0.3 * spo2);

    // Clamp SpO2
    if (filteredSpO2 > 100)
      filteredSpO2 = 100;

    if (filteredSpO2 < 0)
      filteredSpO2 = 0;
  }

  // Reject invalid BPM
  if (beatAvg < 50 || beatAvg > 120) {
    beatAvg = 0;
  }
}

  // ---------- MPU6050 ----------
  ax = read16(0x3B);
  ay = read16(0x3D);
  az = read16(0x3F);

  // ---------- DHT22 ----------
  if (millis() - lastDHTRead > 2000) {

    lastDHTRead = millis();

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
  }

  // ---------- PRINT EVERY 5 SECONDS ----------
  if (millis() - lastPrint > 5000) {

    lastPrint = millis();

    packetID++;
    txTime = millis();

    String csvData =
      String(packetID) + "," +
      String(txTime) + "," +
      String(beatAvg) + "," +
      String(filteredSpO2, 1) + "," +
      String(temperature, 1) + "," +
      String(humidity, 1) + "," +
      String(ax) + "," +
      String(ay) + "," +
      String(az) + "," +
      String(latitude, 6) + "," +
      String(longitude, 6);

    // Print locally
    Serial.println(csvData);

    // Send through LoRa
    LoRa.beginPacket();
    LoRa.print(csvData);
    LoRa.endPacket();

    Serial.println("Packet Sent");

// ---------- Wait for ACK ----------
ackStart = micros();

while (micros() - ackStart < 1000000) {   // Wait up to 1 second

  int packetSize = LoRa.parsePacket();

  if (packetSize) {

    String ack = "";

    while (LoRa.available()) {
      ack += (char)LoRa.read();
    }

    if (ack == ("ACK," + String(packetID))) {

      rtt = micros() - ackStart;
      latency = rtt / 2.0;

      Serial.print("ACK Received | RTT = ");
      Serial.print(rtt);
      Serial.print(" us | One-way Latency = ");
      Serial.print(latency);
      Serial.println(" us");

      break;
    }
  }
}

  }
}