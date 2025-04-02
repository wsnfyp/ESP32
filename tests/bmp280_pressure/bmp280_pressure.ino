#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp;
void setup() {
  Serial.begin(9600);
  if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
    Serial.println("Device not fuond");
    while (1) delay(10);
  }
}

void loop() {
    Serial.print(F("Pressure = "));
    Serial.print(bmp.readPressure()/100);
    Serial.println("hPa");
    Serial.println();
    delay(2000);
}