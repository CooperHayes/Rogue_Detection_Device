#include <WiFi.h>
#include <esp_netif.h>
#include <lwip/etharp.h>
#include <lwip/ip4_addr.h>
#include <lwip/netif.h>
#include <lwip/tcpip.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>



struct ArpRequestData{
    ip4_addr_t localAddress;
    ip4_addr_t targetAddress;
};





unsigned long arpRequestTime = 0;
unsigned long lastLookup = 0;
bool arpLookupPending = false;

void discoverDevice(){

    IPAddress targetIP(192 , 168 , 0 , 69);

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
        return;
    }
    Serial.println("WiFi station interface found.");


    ArpRequestData* request = new ArpRequestData;

   

    IP4_ADDR(&request -> targetAddress , 192 , 168 , 0 , 69);

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
        delete request;
    }

} // end checkNetworkInterface

void findLwipInterface(void* argument){

    ArpRequestData* request = static_cast<ArpRequestData*>(argument);

    bool interfaceFound = false;
    bool requestSucceeded = false;

    for(struct netif* current = netif_list ; current != nullptr ; current = current-> next){
        
        if(ip4_addr_cmp(netif_ip4_addr(current) , &request -> localAddress)){

            Serial.println("Matching interface found.");

            interfaceFound = true;

            struct netif* lwipInterface = current;

            Serial.print("Interface number: ");
            Serial.println(lwipInterface->num);

            err_t arpRequest = etharp_request(
                lwipInterface,
                &request -> targetAddress
            );
            
            requestSucceeded = (arpRequest == ERR_OK);
            if(requestSucceeded){
                Serial.println("ARP request submitted successfully.");
            }
            else{
                Serial.print("ARP request failed. Error code: ");
                Serial.println(static_cast<int>(arpRequest));
            }
           


            break;
        }
    }
    if(arpResultQueue != nullptr){
         BaseType_t status = xQueueSend(arpResultQueue , &requestSucceeded , 0);
         if(status !=pdTRUE){
            Serial.println("Failed to send result to ARP queue.");
         }
    }
    else{
        Serial.println("ARP queue not available.");
    }
    if(interfaceFound == false){
        Serial.println("No matching interface found.");
    }
        
    delete request;
} // end findLwipInterface


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
        Serial.println("Received ARP request result");
        if(receivedResult){
            arpRequestTime = millis();
            arpLookupPending = true;
            Serial.println("Lookup timer start");
        }
        else{
        Serial.println("ARP request was not accepted");
        }
    }  

    if(arpLookupPending && millis() - arpRequestTime >= 2000 && millis() - lastLookup > 1000){
        Serial.println("Ready to start ARP lookup");
        lastLookup = millis();
        bool lookupScheduled = scheduleArpLookup();

        if(lookupScheduled) {
            arpLookupPending = false;
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
        192,
        168,
        0,
        69
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

void lookupArpTable(void* argument){
    ArpRequestData* request = 
        static_cast<ArpRequestData*>(argument);
    
        ArpLookupResult lookupResult = {};

    bool interfaceFound = false;

    for(struct netif* current = netif_list ; current != nullptr ; current = current-> next){
        
        if(ip4_addr_cmp(netif_ip4_addr(current) , &request -> localAddress)){

            interfaceFound = true;
            struct netif* lwipInterface = current;
            Serial.println("Matching interface found for ARP lookup.");
            struct eth_addr* foundMAC = nullptr;
            const ip4_addr_t* foundIP = nullptr;

            s8_t result = etharp_find_addr(
                lwipInterface,
                &request -> targetAddress,
                &foundMAC,
                &foundIP
            );
            if(result >= 0 && foundMAC != nullptr) {

                Serial.println("MAC address successfully resolved!");
                lookupResult.success = true;
                for(int i = 0 ; i < 6 ; i++){
                    lookupResult.mac[i] = foundMAC->addr[i];
                }
            }
            else {
                Serial.println("No resolved ARP entry found for target IP.");
                }
            break;
        
    }
}

    if (!interfaceFound) {
        Serial.println("No matching lwIP interface found for ARP lookup.");
    }

    if(arpLookupQueue != nullptr){
        BaseType_t queueStatus = xQueueSend(
            arpLookupQueue,
            &lookupResult,
            0
        );
        if(queueStatus != pdTRUE){
            Serial.println("Failed to send ARP look up result.");
        }
    }
    else{
        Serial.println("ARP look up queue not available.");
    }
    delete request;
    
} // end lookupArpTable


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

        Serial.println("Device successfully discovered!");

        Serial.printf(
            "MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
            receivedResult.mac[0],
            receivedResult.mac[1],
            receivedResult.mac[2],
            receivedResult.mac[3],
            receivedResult.mac[4],
            receivedResult.mac[5]
        );

    } else {

        Serial.println("Discovery unsuccessful.");
        Serial.println("No resolved MAC address was found.");
    }
} // end processLookupResult
