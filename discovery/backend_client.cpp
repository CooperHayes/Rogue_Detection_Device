#include "backend_client.h"
#include "Arp_discovery.h"
#include "secrets.h"

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

void sendTestScanToBackend() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected.");
        return;
    }

 
    String payload = "{\"devices\":[";
    // Loop through all discovered devices in one subnet scan and construct a JSON payload
    for (int i = 0; i < discoveredDevicesCount; i++) {

        const DiscoveredDevice& device = DiscoveredDevices[i];

        // Convert IP address to text.
        String ip = device.ipAddress.toString();

        // Convert MAC bytes to formatted text.
        char macString[18];

        snprintf(
            macString,
            sizeof(macString),
            "%02X:%02X:%02X:%02X:%02X:%02X",
            device.macAddress[0],
            device.macAddress[1],
            device.macAddress[2],
            device.macAddress[3],
            device.macAddress[4],
            device.macAddress[5]
        );

        // Separate objects with commas.
        if (i > 0) {
            payload += ",";
        }

        // Append one discovered device.
        payload += "{\"ip\":\"";
        payload += ip;
        payload += "\",\"mac\":\"";
        payload += macString;
        payload += "\"}";
    }

    payload += "]}";

    // Set up the HTTP connection.
    WiFiClient client;
    HTTPClient http;

    String serverURL =
        String("http://") + LINUX_VM_IP + ":8000/scan";

    if (!http.begin(client, serverURL)) {
        Serial.println("Failed to initialize HTTP request.");
        return;
    }

    http.setTimeout(5000);
    http.addHeader("Content-Type", "application/json");

    Serial.println("Sending JSON:");
    Serial.println(payload);

    int httpCode = http.POST(payload);

    if (httpCode > 0) {
        Serial.printf("HTTP status: %d\n", httpCode);
        Serial.println(http.getString());
    } else {
        Serial.printf(
            "HTTP error: %s\n",
            http.errorToString(httpCode).c_str()
        );
    }

    http.end();
}