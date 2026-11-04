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

// Discovered device object for devices with a MAC address
struct DiscoveredDevice {
    IPAddress ipAddress;
    uint8_t macAddress[6];
};

// Scan state object for sequential ARP lookups
enum class ScanState{
    IDLE, 
    DISCOVERING,
    COMPLETE
};

// Shared variables for updating discovered devices array in processLookupResult()
constexpr int MAX_DISCOVERED_DEVICES = 50;
extern DiscoveredDevice DiscoveredDevices[MAX_DISCOVERED_DEVICES];
extern int discoveredDevicesCount;

// Shared queues
extern QueueHandle_t arpResultQueue;
extern QueueHandle_t arpLookupQueue;

// Discovery functions
void discoverDevice(IPAddress targetIP);
bool isOnSubnet(IPAddress targetIP);
void checkNetworkInterface();
void startSubnetScan();
void processSubnetScan();
void processArpResult();
bool scheduleArpLookup();
void processLookupResult();
void printDiscoveredDevices();
void processContinuousScan();

// lwIP callback functions
void findLwipInterface(void* argument);
void lookupArpTable(void* argument);

#endif