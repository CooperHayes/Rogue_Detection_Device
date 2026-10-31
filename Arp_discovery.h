#ifndef ARP_DISCOVERY_H
#define ARP_DISCOVERY_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <lwip/ip4_addr.h>



// Data passed into lwIP callbacks
struct ArpRequestData {
    ip4_addr_t localAddress;
    ip4_addr_t targetAddress;
};

// Result returned from an ARP table lookup
struct ArpLookupResult {
    bool success;
    ip4_addr_t targetIP;
    uint8_t mac[6];
};

// Shared queues
extern QueueHandle_t arpResultQueue;
extern QueueHandle_t arpLookupQueue;

// Discovery functions
void discoverDevice(IPAddress targetIP);
bool isOnSubnet(IPAddress targetIP);
void checkNetworkInterface();
void processArpResult();
bool scheduleArpLookup();
void processLookupResult();

// lwIP callback functions
void findLwipInterface(void* argument);
void lookupArpTable(void* argument);

#endif