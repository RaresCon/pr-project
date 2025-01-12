#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <sensor-protocol.hpp>

static Adafruit_BMP280 bmp_sensor;
static float temp_tresh = 30;
static float pres_tresh = 105000;
static float air_tresh = 100;

void setup_bmp()
{
    if (!bmp_sensor.begin(0x76))
        Serial.println("*** Failed to init BMP280 ***");
}

bool populate_sensor_data(sensor_msg *msg)
{
    bool res = false;

    msg->temp = bmp_sensor.readTemperature();
    if (msg->temp > temp_tresh) {
        msg->temp_type = CRITICAL;
        res = true;
    }
    msg->pres = bmp_sensor.readPressure();
    if (msg->pres > pres_tresh) {
        msg->pres_type = CRITICAL;
        res = true;
    }
    msg->aq = 50;
    if (msg->aq > air_tresh) {
        msg->aq_type = CRITICAL;
        res = true;
    }
    return res;
}
