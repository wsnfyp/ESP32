#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP 5

RTC_DATA_ATTR float temp = 1;
RTC_DATA_ATTR float inc = 0.5;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  LoRa.setPins(2, 25, 26);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  temp = temp + inc;
  Serial.print("Temperature: ");
  Serial.println(temp);
  
  LoRa.beginPacket();
  LoRa.print("{Temperature: }");
  LoRa.print(temp);
  LoRa.endPacket();
  delay(1000);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Start deep sleep");
  Serial.flush();
  esp_deep_sleep_start();
}

void loop() {
}