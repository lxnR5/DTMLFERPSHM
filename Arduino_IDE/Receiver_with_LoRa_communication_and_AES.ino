#include <SPI.h>
#include <LoRa.h>
#include <AESLib.h>

#define SS    5
#define RST   14
#define DIO0  26

AESLib aesLib;

byte aes_key[] = {
  0x53, 0x4F, 0x4C, 0x44,
  0x49, 0x45, 0x52, 0x54,
  0x45, 0x41, 0x4D, 0x32,
  0x30, 0x32, 0x36, 0x21
};

byte aes_iv[16] = {
  0x01, 0x02, 0x03, 0x04,
  0x05, 0x06, 0x07, 0x08,
  0x09, 0x0A, 0x0B, 0x0C,
  0x0D, 0x0E, 0x0F, 0x10
};

unsigned char ciphertext[300];
unsigned char cleartext[300];

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

    int index = 0;

    while (LoRa.available()) {
      ciphertext[index++] = LoRa.read();
    }

    byte dec_iv[16];
    memcpy(dec_iv, aes_iv, sizeof(aes_iv));

    int decryptedLength = aesLib.decrypt(
      ciphertext,
      index,
      cleartext,
      aes_key,
      sizeof(aes_key),
      dec_iv
    );

    cleartext[decryptedLength] = '\0';

    Serial.println((char*)cleartext);
  }
}