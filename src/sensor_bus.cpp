#include "sensor_bus.h"

namespace
{
    SemaphoreHandle_t createBinarySemaphore()
    {
        SemaphoreHandle_t handle = xSemaphoreCreateBinary();
        if (handle != nullptr)
        {
            xSemaphoreGive(handle); // start released so first take succeeds
            xSemaphoreTake(handle, 0); // reset to 0 to avoid spurious release
        }
        return handle;
    }

    void clearAndGive(SemaphoreHandle_t handle)
    {
        if (handle == nullptr)
        {
            return;
        }
        xSemaphoreTake(handle, 0);
        xSemaphoreGive(handle);
    }
}

bool sensor_bus_init(SensorBus *bus)
{
    if (bus == nullptr)
    {
        return false;
    }

    bus->readingQueue = xQueueCreate(1, sizeof(SensorReading));
    bus->publishMutex = xSemaphoreCreateMutex();

    if (bus->readingQueue == nullptr || bus->publishMutex == nullptr)
    {
        return false;
    }

    bus->stateSemaphores[static_cast<uint8_t>(SensorDisplayState::NORMAL)] = createBinarySemaphore();
    bus->stateSemaphores[static_cast<uint8_t>(SensorDisplayState::WARNING)] = createBinarySemaphore();
    bus->stateSemaphores[static_cast<uint8_t>(SensorDisplayState::CRITICAL)] = createBinarySemaphore();

    for (int i = 0; i < 3; ++i)
    {
        if (bus->stateSemaphores[i] == nullptr)
        {
            return false;
        }
    }

    return true;
}

bool sensor_bus_publish(SensorBus *bus, const SensorReading &reading, TickType_t timeout)
{
    if (bus == nullptr || bus->readingQueue == nullptr || bus->publishMutex == nullptr)
    {
        return false;
    }

    if (xSemaphoreTake(bus->publishMutex, timeout) != pdTRUE)
    {
        return false;
    }

    BaseType_t status = xQueueOverwrite(bus->readingQueue, &reading);
    xSemaphoreGive(bus->publishMutex);

    if (status != pdPASS)
    {
        return false;
    }

    SensorDisplayState state = reading.state;
    SemaphoreHandle_t stateSemaphore = sensor_bus_state_semaphore(bus, state);
    clearAndGive(stateSemaphore);

    return true;
}

bool sensor_bus_peek(SensorBus *bus, SensorReading &outReading, TickType_t timeout)
{
    if (bus == nullptr || bus->readingQueue == nullptr)
    {
        return false;
    }

    BaseType_t status = xQueuePeek(bus->readingQueue, &outReading, timeout);
    return status == pdPASS;
}

SemaphoreHandle_t sensor_bus_state_semaphore(SensorBus *bus, SensorDisplayState state)
{
    if (bus == nullptr)
    {
        return nullptr;
    }
    return bus->stateSemaphores[static_cast<uint8_t>(state)];
}

