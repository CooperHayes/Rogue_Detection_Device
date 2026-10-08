#include <WiFi.h>
#include "secrets.h"


void setupWiFi(){
    Serial.begin(9600)
    WiFi.begin(WIFI_SSID , WIFI_PASSWORD);

    Serial.print("Connecting to WiFi");

    while(WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Connected!");

}