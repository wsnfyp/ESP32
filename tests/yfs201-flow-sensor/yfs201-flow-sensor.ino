#include <Arduino.h>

#define FLOWSENSOR_PIN 32

volatile unsigned long pulseCount = 0;
double flowRate;
unsigned long currentTime;
unsigned long lastTime;

void IRAM_ATTR pulse() {
    pulseCount++;
}

void setup() {
    pinMode(FLOWSENSOR_PIN, INPUT);
    Serial.begin(115200);
    attachInterrupt(digitalPinToInterrupt(FLOWSENSOR_PIN), pulse, RISING);
    currentTime = millis();
    lastTime = currentTime;
}

void loop() {
    currentTime = millis();
    if (currentTime - lastTime >= 1000) {
        lastTime = currentTime;
        flowRate = (pulseCount / 7.5);
        pulseCount = 0;
        Serial.print(flowRate);
        Serial.println(" L/Min");
    }
}
