#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <Adafruit_NeoPixel.h>

// Struct for sensor data (replaces glob_temperature and glob_humidity)
typedef struct {
    float temperature;
    float humidity;
} SensorData;

// Struct for min/max sensor tracking 
typedef struct {
    float temp_min;
    float temp_max;
    float humi_min;
    float humi_max;
} SensorMinMax;

// Display state enum for LCD
typedef enum {
    DISPLAY_STATE_NORMAL = 0,   // Normal conditions
    DISPLAY_STATE_WARNING,      // Warning: approaching thresholds
    DISPLAY_STATE_CRITICAL      // Critical: exceeded thresholds
} DisplayState;

// Shared sensor data
extern SensorData sharedSensorData;
extern SensorMinMax sharedSensorMinMax;
extern DisplayState currentDisplayState;

extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

extern boolean isWifiConnected;

extern SemaphoreHandle_t xBinarySemaphoreInternet;
extern SemaphoreHandle_t xSensorDataMutex;
extern SemaphoreHandle_t xSensorMinMaxMutex;
extern SemaphoreHandle_t xDataReadySemaphore;
extern SemaphoreHandle_t xSerialMutex;

// Thread-safe Serial macros with mutex
#define SERIAL_PRINT(x) do {\
    if (xSerialMutex != NULL) {\
        if (xSemaphoreTake(xSerialMutex, pdMS_TO_TICKS(100)) == pdTRUE) {\
            Serial.print(x);\
            xSemaphoreGive(xSerialMutex);\
        }\
    } else {\
        Serial.print(x);\
    }\
} while(0)

#define SERIAL_PRINTLN(x) do {\
    if (xSerialMutex != NULL) {\
        if (xSemaphoreTake(xSerialMutex, pdMS_TO_TICKS(100)) == pdTRUE) {\
            Serial.println(x);\
            xSemaphoreGive(xSerialMutex);\
        }\
    } else {\
        Serial.println(x);\
    }\
} while(0)

#define SERIAL_PRINTF(...) do {\
    if (xSerialMutex != NULL) {\
        if (xSemaphoreTake(xSerialMutex, pdMS_TO_TICKS(100)) == pdTRUE) {\
            Serial.printf(__VA_ARGS__);\
            xSemaphoreGive(xSerialMutex);\
        }\
    } else {\
        Serial.printf(__VA_ARGS__);\
    }\
} while(0)

extern Adafruit_NeoPixel strip;
extern bool isLedManualMode;
extern bool isNeoManualMode;
extern bool isMotorManualMode;
extern bool isRelayManualMode;

#endif