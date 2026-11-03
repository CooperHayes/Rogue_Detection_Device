#include "Arp_discovery.h"

#include <lwip/tcpip.h>
#include <WiFi.h>
#include <esp_netif.h>




// Individual ARP look up variables
unsigned long arpRequestTime = 0;
unsigned long lastLookup = 0;
bool arpLookupPending = false;

// Subnet scanning variables
static ScanState scanState = ScanState::IDLE;
static uint8_t currentHost = 9;
static uint8_t finalHost = 14;
static bool lookupResultReceived = false;


// Look up attempt variables
static uint8_t lookupAttempts = 0;
static const uint8_t MAX_LOOKUP_ATTEMPTS = 3;


static IPAddress discoveryTarget;

void startSubnetScan() {

    if (scanState != ScanState::IDLE) {
        Serial.println("Scanner is already running.");
        return;
    }

    scanState = ScanState::DISCOVERING;

    Serial.println("Starting subnet scan.");

    discoverDevice(IPAddress(192, 168, 0, currentHost));
}

void discoverDevice(IPAddress targetIP){

    discoveryTarget = targetIP;

    Serial.println("\n---ARP Disovery Test---");

    if(WiFi.status() != WL_CONNECTED){
        Serial.println("ESP32 is not connected to WiFi");
        return;
    }

    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());

    Serial.print("Subnet Mask: ");
    Serial.println(WiFi.subnetMask());

    Serial.print("Default Gateway: ");
    Serial.println(WiFi.gatewayIP());


    Serial.print("Target IP: ");
    Serial.print(targetIP);

    if(isOnSubnet(targetIP) == false){
        Serial.println("\nTarget IP is not on the local subnet.");
        return;
    }

    Serial.println("\nTarget is on the local subnet");
    Serial.println("Ready to initialize ARP resolution");

    checkNetworkInterface();


} // end discoverDevice

bool isOnSubnet(IPAddress  targetIP){

    IPAddress localIP = WiFi.localIP();
    IPAddress subnetMask = WiFi.subnetMask();

    for(int i = 0 ; i < 4 ; i++){
        
        if((localIP[i] & subnetMask[i]) != (targetIP[i] & subnetMask[i])){

            return false;

        }
    }

    return true;

} // end isOnSubnet

void checkNetworkInterface(){
    if(WiFi.status() != WL_CONNECTED){
        Serial.println("WiFi is not connected.");
        return;
    }
    Serial.println("---NETWORK INTERFACE CHECK---");

    esp_netif_t* wifiInterface = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

    if(wifiInterface == nullptr){
        Serial.println("WiFi interface not located.");
        lookupResultReceived = true;
        return;
    }
    Serial.println("WiFi station interface found.");


    ArpRequestData* request = new ArpRequestData;

   

    IP4_ADDR(&request -> targetAddress , 
        discoveryTarget[0], 
        discoveryTarget[1],
        discoveryTarget[2],
        discoveryTarget[3]
        );

    IPAddress esp32IP = WiFi.localIP();

    IP4_ADDR(
        &request -> localAddress,
        esp32IP[0],
        esp32IP[1],
        esp32IP[2],
        esp32IP[3]
        
    );

    Serial.println("Local and target IPv4 address intialized.");


    err_t result = tcpip_callback(findLwipInterface , request);

    if(result != ERR_OK){
        Serial.println("Failed to schedule interface lookup");
        lookupResultReceived = true;
        delete request;
    }

} // end checkNetworkInterface

void processArpResult(){
    if(arpResultQueue == nullptr){
        Serial.println("Failed to create ARP result queue");
        return;
    }

    bool receivedResult;
    BaseType_t status = xQueueReceive(
        arpResultQueue,
        &receivedResult,
        0
    );

    if(status == pdTRUE){
        Serial.println("---ARP DISCOVERY TEST---");
        Serial.println("Received ARP request result");
        if(receivedResult){
            arpRequestTime = millis();
            arpLookupPending = true;
            Serial.println("Lookup timer start");
        }
        else{
        Serial.println("ARP request was not accepted");
        lookupResultReceived = true;
        }
    }  

    if(arpLookupPending && millis() - arpRequestTime >= 2000 && millis() - lastLookup > 1000){
        Serial.println("Ready to start ARP lookup");
        lastLookup = millis();
        bool lookupScheduled = scheduleArpLookup();

        if(lookupScheduled) {
            arpLookupPending = false;
            lookupAttempts = 0;
        }
        else {
        lookupAttempts++;

        if (lookupAttempts >= MAX_LOOKUP_ATTEMPTS) {
            Serial.println("Maximum lookup attempts reached.");

            arpLookupPending = false;
            lookupAttempts = 0;

            lookupResultReceived = true;
        }
    }
    }
} // end processArpResult

bool scheduleArpLookup(){
    ArpRequestData* request = new ArpRequestData;

    IPAddress esp32IP = WiFi.localIP();

    IP4_ADDR(
        &request->localAddress,
        esp32IP[0],
        esp32IP[1],
        esp32IP[2],
        esp32IP[3]
    );

    IP4_ADDR(
        &request->targetAddress,
        discoveryTarget[0],
        discoveryTarget[1],
        discoveryTarget[2],
        discoveryTarget[3]
    );

    err_t result = tcpip_callback(lookupArpTable , request);

    if(result == ERR_OK){
        Serial.println("ARP lookup successfully scheduled.");
        return true;
    }
    else{
        Serial.println("Failed to schedule ARP lookup.");
        delete request;
        return false;
    }
} // end scheduleArpLookup

void processLookupResult(){

    if(arpLookupQueue == nullptr){
        return;
    }
    ArpLookupResult receivedResult = {};

    BaseType_t status = xQueueReceive(
        arpLookupQueue,
        &receivedResult,
        0
    );

    if(status != pdTRUE){
        return;
    }

    Serial.println("\n--- ARP LOOKUP RESULT ---");

    if (receivedResult.success) {
        if(discoveredDevicesCount < MAX_DISCOVERED_DEVICES){
            DiscoveredDevice device;

            device.ipAddress = IPAddress(
                ip4_addr1(&lookupResult.targetIP),
                ip4_addr2(&lookupResult.targetIP),
                ip4_addr3(&lookupResult.targetIP),
                ip4_addr4(&lookupResult.targetIP)
            );

            memcpy(
                device.macAddress,
                lookupResult.mac,
                6
            );
            
            discoveredDevices[discoveredDevicesCount] = device;

            discoveredDevicesCount = discoveredDevicesCount + 1;

            Serial.println("Device added to discovered devices array.");
        }
        else{
            Serial.print("Discovered devices array is full.");
        }


    } 
    else {

        Serial.println("Discovery unsuccessful.");
        Serial.println("No resolved MAC address was found.");
    }

    lookupResultReceived = true;
} // end processLookupResult

void processSubnetScan() {

    // Only proceed if a scan is running.
    if (scanState != ScanState::DISCOVERING) {
        return;
    }

    // Wait until the current IP lookup finishes.
    if (lookupResultReceived == false) {
        return;
    }

    // Reset the flag for the next IP.
    lookupResultReceived = false;
    lookupAttempts = 0;
    arpLookupPending = false;
    // Move to the next IP address.
    currentHost++;

    // Check whether we have reached the end of the scan.
    if (currentHost > finalHost) {
        scanState = ScanState::COMPLETE;
        Serial.println("\nSubnet scan complete!");
        return;
    }

    // Start discovering the next IP.
    Serial.print("\nScanning next IP: 192.168.0.");
    Serial.println(currentHost);

    discoverDevice(IPAddress(192, 168, 0, currentHost));
} // end processSubnetScan
