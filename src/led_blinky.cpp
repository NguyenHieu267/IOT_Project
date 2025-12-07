#include "led_blinky.h"
#include "global.h"

// Private helper functions for different blink patterns
static void blinkCool();
static void blinkComfort();
static void blinkHot();

void led_blinky(void *pvParameters)
{
    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LOW);

    for (;;)
    {
        if (isLedManualMode) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        xSemaphoreTake(xSensorDataMutex, portMAX_DELAY);
        float temp = sharedSensorData.temperature;
        xSemaphoreGive(xSensorDataMutex);

        // ----- Condition handling: 3 different LED behaviors -----        
        if (temp < 25.0f)
        {
            // COOL RANGE: T < 25°C
            // Slow, calm blink pattern
            blinkCool();
        }
        else if (temp < 35.0f)
        {
            // COMFORT RANGE: 25°C ≤ T < 35°C
            // Medium speed blink pattern
            blinkComfort();
        }
        else
        {
            // HOT RANGE: T ≥ 35°C
            // Fast "alarm" blink pattern to warn about overheating
            blinkHot();
        }
    }
}

// --------------------- Blink pattern implementations ---------------------

// COOL: 2 slow pulses (1 s ON, 1 s OFF)
static void blinkCool()
{
    for (int i = 0; i < 2; ++i)
    {
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1 s ON
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1 s OFF
    }
}

// COMFORT: 4 medium pulses (400 ms ON, 400 ms OFF)
static void blinkComfort()
{
    for (int i = 0; i < 4; ++i)
    {
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(400));   // 0.4 s ON
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(400));   // 0.4 s OFF
    }
}

// HOT: 10 fast pulses (100 ms ON, 100 ms OFF) as alarm
static void blinkHot()
{
    for (int i = 0; i < 10; ++i)
    {
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(100));   // 0.1 s ON
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(100));   // 0.1 s OFF
    }

    // Optional pause to make the "alarm burst" recognizable
    vTaskDelay(pdMS_TO_TICKS(500));
}