#include <WiFi.h>
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

    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("Subnet Mask: ");
    Serial.println(WiFi.subnetMask());

    Serial.print("Default Gateway: ");
    Serial.println(WiFi.gatewayIP());

    Serial.print("Target IP: ");
    Serial.print(targetIP);

    if(isOnSubnet(targetIP) == false){
        Serial.println("Target IP is not on the local subnet.");
        return;
    }

    Serial.println("Target is on the local subnet");
    Serial.println("Ready to initialize ARP resolution");


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