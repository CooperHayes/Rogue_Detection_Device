#include "subnet_scanner.h"
#include "Arp_discovery.h"
#include "backend_client.h"

#include <Arduino.h>
#include <WiFi.h>

// Subnet scanner state
static ScanState scanState = ScanState::IDLE;

static uint8_t currentHost = 1;
static uint8_t finalHost = 10;

static const unsigned long SCAN_INTERVAL = 30000;
static unsigned long lastScanTime = 0;


void startSubnetScan() {

    if (scanState != ScanState::IDLE) {
        Serial.println("Scanner is already running.");
        return;
    }

    scanState = ScanState::DISCOVERING;

    Serial.println("Starting subnet scan.");

    discoverDevice(IPAddress(192, 168, 0, currentHost));
} // end startSubnetScan

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
        printDiscoveredDevices();
        lastScanTime = millis();
        sendTestScanToBackend();
        return;
    }

    // Start discovering the next IP.
    Serial.print("\nScanning next IP: 192.168.0.");
    Serial.println(currentHost);

    discoverDevice(IPAddress(192, 168, 0, currentHost));
} // end processSubnetScan

void printDiscoveredDevices() {
    Serial.println("\n===== DISCOVERED DEVICES =====");

    if(discoveredDevicesCount == 0) {
        Serial.println("No devices were discovered.");
        return;
    }

    for(int i = 0; i < discoveredDevicesCount; i++) {
        Serial.print("\nDevice ");
        Serial.println(i + 1);

        Serial.print("IP Address: ");
        Serial.println(DiscoveredDevices[i].ipAddress);

        Serial.print("MAC Address: ");

        for(int j = 0; j < 6; j++) {
            if(j > 0) {
                Serial.print(":");
            }

            Serial.printf("%02X", DiscoveredDevices[i].macAddress[j]);
        }

        Serial.println();
    }

    Serial.println("\n==============================");
} // end printDiscoveredDevices()


void processContinuousScan() {
    if(scanState == ScanState::COMPLETE &&
       millis() - lastScanTime >= SCAN_INTERVAL) {

        Serial.println("\nStarting a new subnet scan.");

        // Reset scanning variables
        discoveredDevicesCount = 0;
        currentHost = 1;
        scanState = ScanState::IDLE;

        // Reset IP look up related variables
        lookupResultReceived = false;
        arpLookupPending = false;
        lookupAttempts = 0;

        startSubnetScan();
    }
} // end processContinuousScan()