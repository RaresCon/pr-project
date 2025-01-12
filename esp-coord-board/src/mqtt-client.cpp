#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <esp-coord.hpp>
#include <mqtt-client.hpp>
#include <esp-prereq.hpp>
#include <WiFiClientSecure.h>

WiFiClientSecure espClient;
PubSubClient client(espClient);
static TaskHandle_t mqttTaskHandle;
static const char *topic = "sensor/data";

void mqtt_keep_alive(void *parameter)
{
    while (true) {
        client.loop();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void command_callback(char* topic, byte* payload, unsigned int length) {
    JsonDocument doc;
    deserializeJson(doc, payload);

    manageCommandReq(doc["boardId"], doc["command"]);
}

void setup_mqtt_client()
{
    uint8_t retry = 0;

    espClient.setCACert(CA_cert);
    espClient.setCertificate(ESP_CA_cert);
    espClient.setPrivateKey(ESP_RSA_key);
    client.setServer(mqtt_broker, 8883);

    while (!client.connected() && retry < MAX_RETRY) {
        Serial.println("Waiting for connection to MQTT Broker...");
        if (client.connect("ESP32Client")) {
            Serial.println("Connected to MQTT Broker!");

            client.setCallback(command_callback);
            client.subscribe("sensor/command");

            xTaskCreatePinnedToCore(
                mqtt_keep_alive,
                "MQTT Keep-Alive",
                4096,
                NULL,
                1,
                &mqttTaskHandle,
                1
            );

            return;
        }
        retry += 1;
    }
    Serial.print("Connection to MQTT broker failed. Will continue without ");
    Serial.println("publishing sensor data.");
}

void mqtt_send_sensor_data(char *board_id, sensor_msg data)
{
    JsonDocument doc;
    char jsonBuffer[512];
    char type[15];

    doc["boardId"] = board_id;

    decode_data_type(data.temp_type, type);
    doc["temp_type"] = type;
    doc["temperature"] = data.temp;

    decode_data_type(data.pres_type, type);
    doc["pres_type"] = type;
    doc["pressure"] = data.pres;

    decode_data_type(data.aq_type, type);
    doc["aq_type"] = type;
    doc["airQuality"] = data.aq;

    serializeJson(doc, jsonBuffer);
    if (!client.publish(topic, jsonBuffer))
        Serial.println("Failed to send MQTT message.");
}
