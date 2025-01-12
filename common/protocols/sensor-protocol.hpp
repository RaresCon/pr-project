#pragma once

#include <stdint.h>

enum sensor_data_type { NORMAL, CRITICAL, };

typedef struct {
    sensor_data_type temp_type;
    float temp;
    sensor_data_type pres_type;
    float pres;
    sensor_data_type aq_type;
    float aq;
} sensor_msg;

void decode_data_type(sensor_data_type type, char *dest);
