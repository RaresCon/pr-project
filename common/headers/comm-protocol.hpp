#pragma once

#include <stdint.h>
#include "pair-protocol.hpp"

enum msg_type { PAIR, SENSOR_DATA, COMMAND, };

typedef struct {
    msg_type type;
    union {
        pair_msg pair_data;
    } msg;
} raw_msg;

uint8_t bCastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};