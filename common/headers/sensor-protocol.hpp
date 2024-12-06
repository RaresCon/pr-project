#pragma once

#include <stdint.h>

enum SensorData { NORMAL, CRITICAL, };

typedef struct {
    SensorData type;
    float temp;
    float pres;
} sensor_msg;