#pragma once

#include <stdint.h>
#include <sensor-protocol.hpp>

#define MAX_RETRY 10

void setup_mqtt_client();
void mqtt_send_sensor_data(uint8_t board_id, sensor_msg data);