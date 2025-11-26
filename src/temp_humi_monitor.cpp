#include "temp_humi_monitor.h"

#define TEMP_WARNING_HIGH 32.0f
#define TEMP_WARNING_LOW 20.0f
#define TEMP_CRITICAL_HIGH 35.0f
#define TEMP_CRITICAL_LOW 15.0f
#define HUMI_WARNING_HIGH 70.0f
#define HUMI_WARNING_LOW 30.0f
#define HUMI_CRITICAL_HIGH 80.0f
#define HUMI_CRITICAL_LOW 20.0f
#define STATUS_LED 13

namespace
{
    SensorDisplayState classifyReading(float temperature, float humidity)
    {
        if (temperature >= TEMP_CRITICAL_HIGH || temperature <= TEMP_CRITICAL_LOW ||
            humidity >= HUMI_CRITICAL_HIGH || humidity <= HUMI_CRITICAL_LOW)
        {
            return SensorDisplayState::CRITICAL;
        }

        if (temperature >= TEMP_WARNING_HIGH || temperature <= TEMP_WARNING_LOW ||
            humidity >= HUMI_WARNING_HIGH || humidity <= HUMI_WARNING_LOW)
        {
            return SensorDisplayState::WARNING;
        }

        return SensorDisplayState::NORMAL;
    }

    void showNormal(LiquidCrystal_I2C &lcd, const SensorReading &reading)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(reading.temperature, 1);
        lcd.print(" C");
        lcd.setCursor(0, 1);
        lcd.print("Humi: ");
        lcd.print(reading.humidity, 1);
        lcd.print(" %");
    }

    void showWarning(LiquidCrystal_I2C &lcd, const SensorReading &reading)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WARN T:");
        lcd.print(reading.temperature, 1);
        lcd.setCursor(0, 1);
        lcd.print("WARN H:");
        lcd.print(reading.humidity, 1);
        lcd.print("%");
    }

    void showCritical(LiquidCrystal_I2C &lcd, const SensorReading &reading)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("CRIT T:");
        lcd.print(reading.tempMax, 1);
        lcd.setCursor(0, 1);
        lcd.print("CRIT H:");
        lcd.print(reading.humiMax, 1);
        lcd.print("%");
    }
}

void temp_humi_monitor(void *pvParameters)
{
    SensorBus *bus = static_cast<SensorBus *>(pvParameters);
    if (bus == nullptr)
    {
        Serial.println("[temp_humi_monitor] Missing SensorBus pointer");
        vTaskDelete(nullptr);
    }

    DHT20 dht20;
    dht20.begin();

    pinMode(STATUS_LED, OUTPUT);

    float tempMin = 100.0f;
    float tempMax = -100.0f;
    float humiMin = 100.0f;
    float humiMax = -100.0f;

    while (1)
    {
        dht20.read();
        float temperature = dht20.getTemperature();
        float humidity = dht20.getHumidity();

        if (isnan(temperature) || isnan(humidity))
        {
            Serial.println("Failed to read from DHT sensor!");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        tempMax = max(tempMax, temperature);
        tempMin = min(tempMin, temperature);
        humiMax = max(humiMax, humidity);
        humiMin = min(humiMin, humidity);

        SensorDisplayState state = classifyReading(temperature, humidity);
        digitalWrite(STATUS_LED, state == SensorDisplayState::CRITICAL ? HIGH : LOW);

        SensorReading reading{
            temperature,
            humidity,
            tempMin,
            tempMax,
            humiMin,
            humiMax,
            state};

        if (!sensor_bus_publish(bus, reading, pdMS_TO_TICKS(50)))
        {
            Serial.println("Failed to publish sensor reading");
        }

        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.print(" C | State: ");
        Serial.println(static_cast<int>(state));

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void lcd_display_task(void *pvParameters)
{
    SensorBus *bus = static_cast<SensorBus *>(pvParameters);
    if (bus == nullptr)
    {
        Serial.println("[lcd_display_task] Missing SensorBus pointer");
        vTaskDelete(nullptr);
    }

    LiquidCrystal_I2C lcd(33, 16, 2);
    lcd.begin();
    lcd.backlight();

    SemaphoreHandle_t normalSem = sensor_bus_state_semaphore(bus, SensorDisplayState::NORMAL);
    SemaphoreHandle_t warningSem = sensor_bus_state_semaphore(bus, SensorDisplayState::WARNING);
    SemaphoreHandle_t criticalSem = sensor_bus_state_semaphore(bus, SensorDisplayState::CRITICAL);

    while (1)
    {
        SensorReading reading{};
        sensor_bus_peek(bus, reading, portMAX_DELAY);

        SensorDisplayState displayState = SensorDisplayState::NORMAL;
        if (criticalSem != nullptr && xSemaphoreTake(criticalSem, 0) == pdTRUE)
        {
            displayState = SensorDisplayState::CRITICAL;
        }
        else if (warningSem != nullptr && xSemaphoreTake(warningSem, 0) == pdTRUE)
        {
            displayState = SensorDisplayState::WARNING;
        }
        else if (normalSem != nullptr)
        {
            xSemaphoreTake(normalSem, portMAX_DELAY);
            displayState = SensorDisplayState::NORMAL;
        }

        switch (displayState)
        {
        case SensorDisplayState::CRITICAL:
            showCritical(lcd, reading);
            break;
        case SensorDisplayState::WARNING:
            showWarning(lcd, reading);
            break;
        default:
            showNormal(lcd, reading);
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}