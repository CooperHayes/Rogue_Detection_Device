void setup(){
    Serial.begin(115200);
    delay(1000);

    Serial.println("ESP32 started!");

    setupWiFi();
    printNetworkInfo();




}

void loop(){





}