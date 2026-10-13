void setup() {
    Serial.begin(115200);
    delay(10000);

    setupWiFi();
}

void loop() {
    discoverDevice();

    delay(5000);
}