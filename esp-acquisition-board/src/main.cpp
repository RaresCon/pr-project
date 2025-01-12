#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp-sender.hpp>
#include <sensors.hpp>

void setup() {
    Serial.begin(115200);
    Serial.println("SLAVE BOARD\n");

    setup_bmp();
    setup_wifi();
    setup_esp_now();
}

void loop() {
}
