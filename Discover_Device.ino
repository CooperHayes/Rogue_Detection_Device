#include <WiFi.h>
#include <esp_netif.h>
#include <lwip/etharp.h>
#include <lwip/ip4_addr.h>
#include <lwip/netif.h>
#include <lwip/tcpip.h>


struct ArpRequestData{
    ip4_addr_t localAddress;
    ipd_addr_t targetAddress;
};

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

}

void findLwipInterface(void* argument){

    ArpRequestData* request = static_cast<ArpRequestData*>(argument);

    for(struct netif* current = netif_list ; current != nullptr ; current = current-> next){
        
        if(ip4_addr_cmp(netif_ip4_addr(current) , &request -> localaddress)){

            Serial.println("Matching interface found.");

            struct netif* lwipInterface = current;

            Serial.print("Interface number: ");
            Serial.println(lwipInterface->num);

            break;
        }
        
    }
    delete request;
}