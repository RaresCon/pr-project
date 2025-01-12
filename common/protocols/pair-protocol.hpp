#pragma once

#include <stdint.h>

enum pairing { PAIR_REQ, PAIR_SLAVE, PAIR_COORD, };

typedef struct {
    pairing type;
    uint8_t peerMac[6];
} pair_msg;
