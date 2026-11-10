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

    if (discoveredDevicesCount == 0) {
        Serial.println("No discovered devices for this test.");
        return;
    }

    // Get the first discovered device.
    const DiscoveredDevice& device = DiscoveredDevices[0];

    String ip = device.ipAddress.toString();

    // Format the MAC address.
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

    // Construct the JSON payload.
    String deviceJson = String("{\"ip\":\"") +
                        ip +
                        "\",\"mac\":\"" +
                        macString +
                        "\"}";

    String payload = String("{\"devices\":[") +
                     deviceJson +
                     "]}";

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