#include <WiFi.h>
#include "secrets.h"


void setupWiFi(){
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    const int maxAttempts = 5;

    for (int attempt = 1; attempt <= maxAttempts; attempt++) {

        Serial.printf(
            "\nWiFi connection attempt %d/%d\n",
            attempt,
            maxAttempts
        );

        WiFi.begin(ssid, password);

        unsigned long startTime = millis();

        // Allow up to 15 seconds for this attempt.
        while (WiFi.status() != WL_CONNECTED &&
               millis() - startTime < 15000) {

            Serial.print("WiFi status: ");
            Serial.println(WiFi.status());

            delay(1000);
        }

        if (WiFi.status() == WL_CONNECTED) {

            Serial.println("\nWiFi connected successfully!");

            Serial.print("ESP32 IP: ");
            Serial.println(WiFi.localIP());

            return;
        }

        Serial.println("Connection attempt failed.");

        WiFi.disconnect();
        delay(2000);
    }

    Serial.println("Unable to connect after five attempts.");
}