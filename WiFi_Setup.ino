#include <WiFi.h>
#include "secrets.h"


void setupWiFi(){
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID , WIFI_PASSWORD);

    Serial.print("Connecting to WiFi");

    while(WiFi.status() != WL_CONNECTED){
       Serial.print("WiFi status: ");
       Serial.println(WiFi.status());
       delay(2000);
    }

    Serial.println();
    Serial.println("Connected!");

}