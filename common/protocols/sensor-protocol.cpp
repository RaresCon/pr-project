#include <string.h>
#include "sensor-protocol.hpp"

void decode_data_type(sensor_data_type type, char *dest)
{
    switch (type) {
        case NORMAL:
            strcpy(dest, "NORMAL");
            break;
        case CRITICAL:
            strcpy(dest, "CRITICAL");
            break;
        default:
            break;
    }
}