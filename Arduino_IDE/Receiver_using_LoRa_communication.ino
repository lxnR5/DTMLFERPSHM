#include <SPI.h>
#include <LoRa.h>

#define SS    5
#define RST   14
#define DIO0  26

// ===== Communication Metrics =====
unsigned long expectedPacketID = 0;
bool firstPacket = true;
unsigned long receivedPackets = 0;
unsigned long lostPackets = 0;

void setup() {

  Serial.begin(115200);

  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa Init Failed!");
    while (1);
  }

  Serial.println("LoRa Receiver Ready!");
}

void loop() {

  int packetSize = LoRa.parsePacket();

  if (packetSize) {

    String receivedData = "";

    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }

    if (receivedData.startsWith("ACK")) {
      return;
    }

    // -------- Parse Packet --------
    int comma1 = receivedData.indexOf(',');
    int comma2 = receivedData.indexOf(',', comma1 + 1);

    unsigned long packetID =
        receivedData.substring(0, comma1).toInt();

    unsigned long txTime =
        receivedData.substring(comma1 + 1, comma2).toInt();

    String sensorData =
        receivedData.substring(comma2 + 1);

// -------- Packet Loss --------
if (firstPacket) {

    // Synchronize with first packet
    expectedPacketID = packetID + 1;
    firstPacket = false;

}
else {

    if (packetID > expectedPacketID) {
        lostPackets += (packetID - expectedPacketID);
    }

    expectedPacketID = packetID + 1;
}

receivedPackets++;

    // -------- Send ACK back to TX --------
LoRa.beginPacket();
LoRa.print("ACK,");
LoRa.print(packetID);
LoRa.endPacket();



    // -------- RSSI & SNR --------
    long rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();

    // -------- Print --------
    Serial.println("--------------------------------");
    Serial.println(receivedData);

    Serial.print("Packet ID : ");
    Serial.println(packetID);

    Serial.print("RSSI : ");
    Serial.print(rssi);
    Serial.println(" dBm");

    Serial.print("SNR : ");
    Serial.print(snr);
    Serial.println(" dB");

    Serial.print("Received : ");
    Serial.println(receivedPackets);

    Serial.print("Lost : ");
    Serial.println(lostPackets);
  }
}