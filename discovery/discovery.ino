#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <Arduino.h>
#include "Arp_discovery.h"
#include "backend_client.h"
#include "subnet_scanner.h"



DiscoveredDevice DiscoveredDevices[MAX_DISCOVERED_DEVICES];

int discoveredDevicesCount = 0;

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


    startSubnetScan();


}

void loop() {
    processArpResult();
    processLookupResult();
    processSubnetScan();
    processContinuousScan();
    delay(100);
}