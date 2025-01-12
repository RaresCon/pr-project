#include <WiFi.h>
#include <esp-coord.hpp>
#include <esp_wifi.h>
#include <pair-protocol.hpp>
#include <comm-protocol.hpp>
#include <mqtt-client.hpp>
#include <esp-prereq.hpp>

static vector<esp_now_peer_info_t> slaves;

void managePairReq(pair_msg msg)
{
    raw_msg res;
    esp_err_t esp_res;
    esp_now_peer_info_t slaveInfo;

    if (msg.type != PAIR_REQ)
        return;

    memset(&slaveInfo, 0, sizeof(slaveInfo));
    memcpy(slaveInfo.peer_addr, msg.peerMac, 6);
    esp_res = esp_now_add_peer(&slaveInfo);
    if (esp_res != ESP_OK && esp_res != ESP_ERR_ESPNOW_EXIST) {
        Serial.println("Failed to add peer during pairing");
        return;
    }

    res.type = PAIR;
    res.msg.pair_data.type = PAIR_COORD;
    esp_wifi_get_mac(WIFI_IF_STA, res.msg.pair_data.peerMac);
    esp_now_send(msg.peerMac, (uint8_t *)&res, sizeof(res));

    if (esp_res != ESP_ERR_ESPNOW_EXIST) {
        slaves.push_back(slaveInfo);
        Serial.println("Pairing with slave successful!");
    }
}

void manageCommandReq(const char *boardId, const char *command)
{
    raw_msg msg;

    msg.type = COMMAND;

    strcpy(msg.board_id, boardId);
    msg.msg.command.type = encode_command(command);
    if (msg.msg.command.type == ERROR) {
        Serial.println("Sending command to slaves failed!");
        return;
    }

    for (esp_now_peer_info_t slave : slaves) {
        Serial.println("Sending command to slaves\n");
        esp_now_send(slave.peer_addr, (uint8_t *)&msg, sizeof(msg));
    }
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    raw_msg *msg = (raw_msg *)incomingData;
    esp_now_peer_info_t slave;

    Serial.print("Incoming MAC Address: ");
    for (int i = 0; i < 6; i++) {
        Serial.print(mac[i], HEX);
        if (i != 5) Serial.print(":");
    }
    Serial.print("\nBoard ID: ");
    Serial.println(msg->board_id);

    switch (msg->type) {
        case PAIR:
            managePairReq(msg->msg.pair_data);
            break;
        case SENSOR_DATA:
            mqtt_send_sensor_data(msg->board_id, msg->msg.sensor_data);
            break;
        default:
            break;
    }

    if (esp_now_get_peer(mac, &slave) == ESP_ERR_ESPNOW_NOT_FOUND) {
        memset(&slave, 0, sizeof(slave));
        memcpy(slave.peer_addr, mac, sizeof(slave.peer_addr));
        esp_now_add_peer(&slave);
        slaves.push_back(slave);
    }
}

void setup_wifi()
{
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.begin(ssid, password);

    while (!WiFi.isConnected()) {
        Serial.println("Waiting for connection...");
        sleep(1);
    }
    Serial.println("Connected.");
    Serial.println(WiFi.localIP());
}

void setup_esp_now()
{
    esp_now_peer_info_t bcastInfo;

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);

    memset(&bcastInfo, 0, sizeof(bcastInfo));
    memcpy(bcastInfo.peer_addr, bCastAddr, sizeof(bCastAddr));
    if (esp_now_add_peer(&bcastInfo) != ESP_OK)
        Serial.println("Failed to add peer.");
}