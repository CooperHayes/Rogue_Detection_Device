#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

QueueHandle_t arpResultQueue = nullptr;



void setup() {
    Serial.begin(115200);
    delay(10000);

    setupWiFi();

    arpResultQueue = xQueueCreate(1 , sizeof(bool));
    if(arpResultQueue == nullptr){
        Serial.println("Failed to create ARP result queue");
        return;
    }

    discoverDevice();


}

void loop() {
    processArpResult();
    delay(100);
}