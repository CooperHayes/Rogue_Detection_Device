#include <WiFi.h>
#include <esp_netif.h>
#include <lwip/etharp.h>
#include <lwip/ip4_addr.h>
#include <lwip/netif.h>

void discoverDevice(){

    IPAddress targetIP(192 , 168 , 0 , 69);

    Serial.println("\n---ARP Disovery Test---");

    while(WiFi.status() != WL_CONNECTED){
        Serial.println("ESP32 is not connected to WiFi");
        return;
    }

    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());

    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());

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
        
        if((localIP[i] & subnetMask[i]) != (localIP[i] & subnetMask[i])){

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
    Serial.print("WiFi station interface found.");



    struct netif* lwipInterface =
        static_cast<struct netif*>(
            esp_netif_get_netif_impl(wifiInterface)
        );

    if(lwipInterface == nullptr){
        Serial.println("lwIP interface not found");
    }

    Serial.println("lwIP interface found!");
    Serial.println("---END INTERFACE CHECK---");
    Serial.println();

}