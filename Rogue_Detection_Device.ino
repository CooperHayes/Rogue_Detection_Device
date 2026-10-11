void setup() {
    Serial.begin(115200);
    delay(10000);

    setupWiFi();
}

void loop() {
    Serial.println("\n--- ESP32 Network Information ---");

    printNetworkInfo();

    Serial.println("-------------------------------");

    delay(5000);
}