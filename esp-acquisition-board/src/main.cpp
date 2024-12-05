#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp-sender.hpp>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp_sensor;

void setup() {
    Serial.begin(115200);
    Serial.println("SLAVE BOARD\n");

    if (!bmp_sensor.begin(0x76))
        Serial.println("*** Failed to init BMP280 ***");
    setup_wifi();
    setup_esp_now();
}

void loop() {
    Serial.print("Temp: ");
    Serial.print(bmp_sensor.readTemperature());
    Serial.println();
    sleep(1);
}
