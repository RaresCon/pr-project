#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <sensor-protocol.hpp>

static Adafruit_BMP280 bmp_sensor;

void setup_bmp()
{
    if (!bmp_sensor.begin(0x76))
        Serial.println("*** Failed to init BMP280 ***");
}

void populate_sensor_data(sensor_msg *msg)
{
    msg->temp = bmp_sensor.readTemperature();
    msg->pres = bmp_sensor.readPressure();
}
