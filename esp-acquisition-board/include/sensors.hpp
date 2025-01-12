#pragma once

#include <stdint.h>
#include <sensor-protocol.hpp>

void setup_bmp();
bool populate_sensor_data(sensor_msg *msg);
