#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

QueueHandle_t arpResultQueue = nullptr;
QueueHandle_t arpLookupQueue = nullptr;

struct ArpLookupResult {
    bool success;
    uint8_t mac[6];
};


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

    discoverDevice();


}

void loop() {
    processArpResult();
    processLookupResult();
    delay(100);
}