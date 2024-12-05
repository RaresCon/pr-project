#pragma once

#include <stdint.h>

enum PairingStatus { NOT_PAIRED, PAIR_REQUEST, PAIR_REQUESTED, PAIR_PAIRED, };
enum Pairing { PAIR_REQ, PAIR_SLAVE, PAIR_COORD, };

typedef struct {
    Pairing type;
    uint8_t peerMac[6];
} pair_msg;
