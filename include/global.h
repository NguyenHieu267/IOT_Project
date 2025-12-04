#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// Struct for sensor data (replaces glob_temperature and glob_humidity)
typedef struct {
    float temperature;
    float humidity;
} SensorData;

// Shared sensor data
extern SensorData sharedSensorData;

extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

extern boolean isWifiConnected;

extern SemaphoreHandle_t xBinarySemaphoreInternet;
extern SemaphoreHandle_t xSensorDataMutex;
#endif