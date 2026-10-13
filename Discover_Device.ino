#include "WiFi.h"

void discoverDevice(){

    IPAddress targetIP(192 , 168 , 0 , 69);

    Serial.println("\n---ARP Disovery Test---")

    while(WiFi.status() != WL_CONNECTED){
        Serial.println("ESP32 is not connected to WiFi");
        return;
    }

    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("Subnet Mask: ");
    Serial.println(WiFi.subnetMask());

    Serial.print("Default Gateway: ");
    Serial.println(WiFi.gatewayIP());

    Serial.print("Target IP: ");
    Serial.print(targetIP);

}