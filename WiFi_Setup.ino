#include <WiFi.h>
#include "secrets.h"


void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("1: ESP32 started");

    setupWiFi();

    Serial.println("2: WiFi function finished");

    printNetworkInfo();

    Serial.println("3: Network information finished");
}

void loop() {
    Serial.println("4: Main loop running");
    delay(2000);
}