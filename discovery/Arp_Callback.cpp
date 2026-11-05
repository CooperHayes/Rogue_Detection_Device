#include "Arp_discovery.h"

#include <WiFi.h>
#include <lwip/etharp.h>
#include <lwip/ip4_addr.h>
#include <lwip/netif.h>
#include <lwip/tcpip.h>



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

void lookupArpTable(void* argument){
    ArpRequestData* request = 
        static_cast<ArpRequestData*>(argument);
    
    ArpLookupResult lookupResult = {};
    lookupResult.targetIP = request -> targetAddress;

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
