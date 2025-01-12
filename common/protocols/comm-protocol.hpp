#pragma once

#include <stdint.h>
#include "pair-protocol.hpp"
#include "sensor-protocol.hpp"
#include "command-protocol.hpp"

enum msg_type { PAIR, SENSOR_DATA, COMMAND, };

typedef struct {
    msg_type type;
    char board_id[33];
    union {
        pair_msg pair_data;
        sensor_msg sensor_data;
        command_msg command;
    } msg;
} raw_msg;

uint8_t bCastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};