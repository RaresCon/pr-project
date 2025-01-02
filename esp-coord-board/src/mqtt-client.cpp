#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <esp-coord.hpp>
#include <mqtt-client.hpp>
#include <esp-prereq.hpp>

WiFiClient espClient;
PubSubClient client(espClient);
static const char *topic = "sensor/data";

void setup_mqtt_client()
{
    uint8_t retry = 0;

    client.setServer(mqtt_broker, 1883);
    while (!client.connected() && retry < MAX_RETRY) {
        Serial.println("Waiting for connection to MQTT Broker...");
        if (client.connect("ESP32Client")) {
            Serial.println("Connected to MQTT Broker!");
            return;
        }
        retry += 1;
    }
    Serial.print("Connection to MQTT broker failed. Will continue without ");
    Serial.println("publishing sensor data.");
}

void mqtt_send_sensor_data(uint8_t board_id, sensor_msg data)
{
    JsonDocument doc;
    char jsonBuffer[512];

    doc["boardId"] = board_id;
    doc["temperature"] = data.temp;
    doc["pressure"] = data.pres;
    serializeJson(doc, jsonBuffer);

    if (!client.publish(topic, jsonBuffer))
        Serial.println("Failed to send MQTT message.");
}
