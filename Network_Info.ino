#include <WiFi.h>
#include "secrets.h"


void printNetworkInfo(){
    Serial.begin(9600);
    Serial.println();
    Serial.println("NETWORK INFO");
    Serial.println("============");
    Serial.println();

    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("Subnet Mask: ");
    Serial.println(WiFi.subnetMask());

    Serial.print("Default Gateway: ");
    Serial.println(WiFi.gatewayIP());

    Serial.print("ESP32 MAC: ");
    Serial.println(WiFi.macAddress());

}