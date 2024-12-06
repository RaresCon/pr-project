#include <WiFi.h>
#include <esp-sender.hpp>
#include <esp_wifi.h>
#include <comm-protocol.hpp>
#include <sensors.hpp>

static bool channelFound = false;
static uint8_t channel = 0;

static Pairing parentType;
static esp_now_peer_info_t parentInfo;
static vector<esp_now_peer_info_t> slaves;

void sendSensorData()
{
    raw_msg msg;
    sensor_msg data;

    populate_sensor_data(&data);

    msg.type = SENSOR_DATA;
    msg.board_id = 10;
    msg.msg.sensor_data = data;

    esp_now_send(parentInfo.peer_addr, (uint8_t *)&msg, sizeof(msg));
}

void addParentInfo(const uint8_t *mac)
{
    memset(&parentInfo, 0, sizeof(parentInfo));
    memcpy(parentInfo.peer_addr, mac, sizeof(parentInfo.peer_addr));
    if (esp_now_add_peer(&parentInfo) != ESP_OK) {
        Serial.println("Failed to add parent while pairing.");
        return;
    }
    Serial.print("Pairing with parent successful | Parent type - ");
    Serial.print(parentType == PAIR_COORD ? "Coordinator" : "Upper Slave");
    Serial.println(".");
}

void managePairReq(pair_msg msg)
{
    switch (msg.type) {
        case PAIR_COORD:
        case PAIR_SLAVE:
            channelFound = true;
            parentType = msg.type;
            addParentInfo(msg.peerMac);
            break;
        case PAIR_REQ:
            break;
        default:
            break;
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
    Serial.println();

    switch (msg->type) {
        case PAIR:
            managePairReq(msg->msg.pair_data);
            break;
        case SENSOR_DATA:
            break;
        case COMMAND:
            break;
        default:
            break;
    }
}

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.print("Trying to a peer on channel ");
    Serial.print(channel % 13);
    Serial.println(".");
}

void tryNextChannel()
{
    channel += 1;
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(channel % 13, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
}

void setup_wifi()
{
    WiFi.mode(WIFI_MODE_STA);
}

void setup_esp_now()
{
    esp_now_peer_info_t bcastInfo;
    raw_msg pairing;

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

    pairing.type = PAIR;
    pairing.board_id = 10;
    pairing.msg.pair_data.type = PAIR_REQ;
    WiFi.macAddress(pairing.msg.pair_data.peerMac);
    while (!channelFound) {
        esp_now_send(bCastAddr, (uint8_t *)&pairing, sizeof(pairing));
        sleep(2);
        tryNextChannel();
    }
    esp_now_unregister_send_cb();
}
