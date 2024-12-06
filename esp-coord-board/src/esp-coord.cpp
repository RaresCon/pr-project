#include <WiFi.h>
#include <esp-coord.hpp>
#include <esp_wifi.h>
#include <pair-protocol.hpp>
#include <comm-protocol.hpp>

static vector<esp_now_peer_info_t> slaves;
static const char* ssid = "TPL_DGRL";
static const char* password = "ccRDGL-1524";

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

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    raw_msg *msg = (raw_msg *)incomingData;

    Serial.print("Incoming MAC Address: ");
    for (int i = 0; i < 6; i++) {
        Serial.print(mac[i]);
        if (i != 5) Serial.print(":");
    }
    Serial.print("\nBoard ID: ");
    Serial.println(msg->board_id);

    switch (msg->type) {
        case PAIR:
            managePairReq(msg->msg.pair_data);
            break;
        case SENSOR_DATA:
            Serial.print("Temp: ");
            Serial.println(msg->msg.sensor_data.temp);
            Serial.print("Pres: ");
            Serial.println(msg->msg.sensor_data.pres);
            break;
        case COMMAND:
            break;
        default:
            break;
    }
}

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.print("Pair response to MAC Address: ");
    for (int i = 0; i < 6; i++) {
        Serial.print(mac_addr[i]);
        if (i != 5) Serial.print(":");
    }
    Serial.println(".");
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
}

void setup_esp_now()
{
    esp_now_peer_info_t bcastInfo;

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(OnDataRecv);

    memset(&bcastInfo, 0, sizeof(bcastInfo));
    memcpy(bcastInfo.peer_addr, bCastAddr, sizeof(bCastAddr));
    if (esp_now_add_peer(&bcastInfo) != ESP_OK)
        Serial.println("Failed to add peer.");
    slaves.push_back(bcastInfo);
}