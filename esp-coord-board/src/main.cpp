#include <Arduino.h>
#include <esp-coord.hpp>
#include <mqtt-client.hpp>
#include <esp-prereq.hpp>

void setup() {
    Serial.begin(baud_rate);
    Serial.println("COORDINATOR BOARD\n");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    setup_wifi();
    setup_mqtt_client();
    setup_esp_now();
}

void loop() {
}
