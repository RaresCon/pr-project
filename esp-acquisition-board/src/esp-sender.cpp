#include <WiFi.h>
#include <esp-sender.hpp>
#include <esp_wifi.h>
#include <comm-protocol.hpp>
#include <sensors.hpp>

static bool channelFound = false;
static uint8_t channel = 0;
static TaskHandle_t sensorTaskHandle;
static TaskHandle_t alarmTaskHandle;
static pairing parentType;
static esp_now_peer_info_t parentInfo;
static vector<esp_now_peer_info_t> slaves;
static uint8_t alarm_pin = GPIO_NUM_14;
static bool do_sensors = true, do_alarm = false, do_alarm_or = false;

#define FORCE_SKIP_COORD
#ifdef FORCE_SKIP_COORD
static char boardId[33] = "Beta";
#else
static char boardId[33] = "Alhpa";
#endif

void sendSensorData(void *parameter)
{
    while (true) {
        raw_msg msg;
        sensor_msg data;
        if (!do_sensors) {
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        memset(&data, 0, sizeof(data));
        do_alarm = populate_sensor_data(&data);

        msg.type = SENSOR_DATA;
        strcpy(msg.board_id, boardId);
        msg.msg.sensor_data = data;

        esp_now_send(parentInfo.peer_addr, (uint8_t *)&msg, sizeof(msg));
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void manageAlarm(void *parameter)
{
    uint8_t mode = LOW;

    pinMode(alarm_pin, OUTPUT);
    digitalWrite(alarm_pin, LOW);

    while (true) {
        if (!do_alarm_or && !do_alarm) {
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        digitalWrite(alarm_pin, mode);
        mode = (++mode) % 2;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
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

void resUpperSlave(pair_msg msg)
{
    raw_msg res;
    esp_err_t esp_res;
    esp_now_peer_info_t lowerSlaveInfo;

    if (msg.type != PAIR_REQ)
        return;

    memset(&lowerSlaveInfo, 0, sizeof(lowerSlaveInfo));
    memcpy(lowerSlaveInfo.peer_addr, msg.peerMac, 6);
    esp_res = esp_now_add_peer(&lowerSlaveInfo);
    if (esp_res != ESP_OK && esp_res != ESP_ERR_ESPNOW_EXIST) {
        Serial.println("Failed to add peer during pairing");
        return;
    }

    res.type = PAIR;
    res.msg.pair_data.type = PAIR_SLAVE;
    esp_wifi_get_mac(WIFI_IF_STA, res.msg.pair_data.peerMac);
    esp_now_send(msg.peerMac, (uint8_t *)&res, sizeof(res));

    if (esp_res != ESP_ERR_ESPNOW_EXIST) {
        slaves.push_back(lowerSlaveInfo);
        Serial.println("Pairing with lower slave successful!");
    }
}

void managePairReq(pair_msg msg)
{
    switch (msg.type) {
        case PAIR_COORD:
        case PAIR_SLAVE:
#ifdef FORCE_SKIP_COORD
            if (msg.type == PAIR_COORD) {
                Serial.println("Skipping Coordinator");
                return;
            }
#endif
            channelFound = true;
            parentType = msg.type;
            addParentInfo(msg.peerMac);
            break;
        case PAIR_REQ:
            resUpperSlave(msg);
            break;
        default:
            break;
    }
}

void manageSlaveData(raw_msg *msg)
{
    Serial.println("Sending data from lower slave");
    if (esp_now_send(parentInfo.peer_addr, (uint8_t *)msg, sizeof(*msg)) != ESP_OK) {
        Serial.println("Error while sending data from lower slave");
    }
}

void manageCommandReq(raw_msg *msg)
{
    if (strcmp(boardId, msg->board_id)) {
        for (esp_now_peer_info_t slave : slaves) {
            Serial.println("Sending command to lower slaves");
            esp_now_send(slave.peer_addr, (uint8_t *)msg, sizeof(*msg));
        }
        return;
    }

    Serial.println(msg->msg.command.type);
    switch (msg->msg.command.type) {
        case DISABLE:
            do_sensors = false;
            break;
        case ENABLE:
            do_sensors = true;
            break;
        case ALARM:
            do_alarm_or = !do_alarm_or;
            break;
        default:
            break;
    }
    Serial.println("Board Status:");
    Serial.print("\tSensors: ");
    Serial.println(do_sensors);
    Serial.print("\tAlarm: ");
    Serial.println(do_alarm);
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    raw_msg *msg = (raw_msg *)incomingData;

    Serial.print("Incoming MAC Address: ");
    for (int i = 0; i < 6; i++) {
        Serial.print(mac[i], HEX);
        if (i != 5) Serial.print(":");
    }
    Serial.println();

    switch (msg->type) {
        case PAIR:
            managePairReq(msg->msg.pair_data);
            break;
        case SENSOR_DATA:
            manageSlaveData(msg);
            break;
        case COMMAND:
            manageCommandReq(msg);
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
    Serial.println(boardId);
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

    pairing.type = PAIR;
    strcpy(pairing.board_id, boardId);
    pairing.msg.pair_data.type = PAIR_REQ;
    WiFi.macAddress(pairing.msg.pair_data.peerMac);
    while (!channelFound) {
        esp_now_send(bCastAddr, (uint8_t *)&pairing, sizeof(pairing));
        sleep(2);
        tryNextChannel();
    }
    esp_now_unregister_send_cb();

    xTaskCreatePinnedToCore(
        sendSensorData,
        "sendSensorData",
        4096,
        NULL,
        1,
        &sensorTaskHandle,
        1
    );
    xTaskCreatePinnedToCore(
        manageAlarm,
        "manageAlarm",
        4096,
        NULL,
        1,
        &alarmTaskHandle,
        1
    );
}
