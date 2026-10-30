#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <Arduino.h>
#include "Arp_discovery.h"


QueueHandle_t arpResultQueue = nullptr;
QueueHandle_t arpLookupQueue = nullptr;



void setup() {
    Serial.begin(115200);
    delay(10000);

    setupWiFi();

    arpResultQueue = xQueueCreate(1 , sizeof(bool));
    if(arpResultQueue == nullptr){
        Serial.println("Failed to create ARP result queue");
        return;
    }

    arpLookupQueue = xQueueCreate(1 , sizeof(ArpLookupResult));
    if(arpLookupQueue == nullptr){
        Serial.println("Failed to create ARP lookup queue");
        return;
    }

    IPAddress laptopIP(192 ,168, 0, 69);
    discoverDevice(laptopIP);


}

void loop() {
    processArpResult();
    processLookupResult();
    delay(100);
}