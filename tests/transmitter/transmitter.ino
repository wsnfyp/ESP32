#include <SPI.h>
#include <LoRa.h>

#define SS 2
#define RST 25
#define DIO0 26

int counter = 0;
void setup() {
  Serial.begin(9600);
  while (!Serial);
  LoRa.setPins(SS, RST, DIO0);
  LoRa.setFrequency(433E6);
  LoRa.setSyncWord(0x39);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setSpreadingFactor(7);
  Serial.println("LoRa Sender");

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;

  delay(5000);
}