#ifndef __SENSOR_BUS_H__
#define __SENSOR_BUS_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

enum class SensorDisplayState : uint8_t
{
    NORMAL = 0,
    WARNING,
    CRITICAL
};

struct SensorReading
{
    float temperature;
    float humidity;
    float tempMin;
    float tempMax;
    float humiMin;
    float humiMax;
    SensorDisplayState state;
};

struct SensorBus
{
    QueueHandle_t readingQueue;
    SemaphoreHandle_t publishMutex;
    SemaphoreHandle_t stateSemaphores[3];
};

bool sensor_bus_init(SensorBus *bus);
bool sensor_bus_publish(SensorBus *bus, const SensorReading &reading, TickType_t timeout = portMAX_DELAY);
bool sensor_bus_peek(SensorBus *bus, SensorReading &outReading, TickType_t timeout = portMAX_DELAY);
SemaphoreHandle_t sensor_bus_state_semaphore(SensorBus *bus, SensorDisplayState state);

#endif

