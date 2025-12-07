#include "temp_humi_monitor.h"
#include "global.h"
#include "task_webserver.h"


// Definitions for 3 display states
#define TEMP_CRITICAL_HIGH  35.0
#define TEMP_CRITICAL_LOW   15.0
#define TEMP_WARNING_HIGH   30.0
#define TEMP_WARNING_LOW    18.0

#define HUMI_CRITICAL_HIGH  80.0
#define HUMI_CRITICAL_LOW   20.0
#define HUMI_WARNING_HIGH   70.0
#define HUMI_WARNING_LOW    30.0

#define LED_ALERT 13

DHT20 dht20;
LiquidCrystal_I2C lcd(33,16,2);

// Reinitialize LCD after relay EMI crash
void reinit_lcd() {
    Wire.begin(11, 12);
    lcd.begin();
    lcd.backlight();
}


// Determine display state based on sensor readings
static DisplayState determineDisplayState(float temp, float humi)
{
    // CRITICAL: values exceed critical thresholds
    if (temp >= TEMP_CRITICAL_HIGH || temp <= TEMP_CRITICAL_LOW ||
        humi >= HUMI_CRITICAL_HIGH || humi <= HUMI_CRITICAL_LOW) {
        return DISPLAY_STATE_CRITICAL;
    }
    
    // WARNING: values in warning range
    if (temp >= TEMP_WARNING_HIGH || temp <= TEMP_WARNING_LOW ||
        humi >= HUMI_WARNING_HIGH || humi <= HUMI_WARNING_LOW) {
        return DISPLAY_STATE_WARNING;
    }
    
    // NORMAL: all values within normal range
    return DISPLAY_STATE_NORMAL;
}

// Display NORMAL state 
static void displayNormalState(float temp, float humi)
{
    lcd.clear();  // ✅ CLEAR SCREEN FIRST!
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temp, 1);
    lcd.print(" C");
    
    lcd.setCursor(0, 1);
    lcd.print("Humi: ");
    lcd.print(humi, 1);
    lcd.print(" %");
    
    digitalWrite(LED_ALERT, LOW);
}

// Display WARNING state (total 3s delay)
static void displayWarningState(float temp, float humi)
{
    // Screen 1: !! WARNING !! (1.5s)
    lcd.clear();  // ✅ CLEAR FIRST
    lcd.setCursor(0, 0);
    lcd.print("!! WARNING !!");
    
    digitalWrite(LED_ALERT, HIGH);
    vTaskDelay(pdMS_TO_TICKS(1500));
    
    // Screen 2: Temp and Humi values (1.5s)
    lcd.clear();  // ✅ CLEAR BEFORE NEW CONTENT
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temp, 1);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Humi: ");
    lcd.print(humi, 1);
    lcd.print(" %");
    
    digitalWrite(LED_ALERT, LOW);
    vTaskDelay(pdMS_TO_TICKS(1500));
}

// Display CRITICAL state 
static void displayCriticalState(float temp, float humi, SensorMinMax *minmax)
{
    // Screen 1: !! CRITICAL !! (1.5s)
    lcd.clear();  // ✅ CLEAR FIRST
    lcd.setCursor(0, 0);
    lcd.print("!! CRITICAL !!");
    
    digitalWrite(LED_ALERT, HIGH);
    vTaskDelay(pdMS_TO_TICKS(1500));
    
    // Screen 2: Temp and Humi values (1.5s)
    lcd.clear();  // ✅ CLEAR BEFORE NEW CONTENT
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temp, 1);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Humi: ");
    lcd.print(humi, 1);
    lcd.print(" %");
    
    digitalWrite(LED_ALERT, LOW);
    vTaskDelay(pdMS_TO_TICKS(1500));
}


void temp_humi_monitor(void *pvParameters)
{
    // Wait for main setup() to complete
    vTaskDelay(pdMS_TO_TICKS(200));
    
    Wire.begin(11, 12);
    dht20.begin();

    lcd.begin();
    lcd.backlight();

    pinMode(LED_ALERT, OUTPUT);

    SERIAL_PRINTLN("Starting Temperature and Humidity Monitor Task...");
    
    DisplayState previousState = DISPLAY_STATE_NORMAL;
    
    while (1) {
        // Read sensor data
        dht20.read();
        float temperature = dht20.getTemperature();
        float humidity = dht20.getHumidity();

        // Error handling: skip this iteration if sensor read fails
        if (isnan(temperature) || isnan(humidity)) {
            SERIAL_PRINTLN("❌ Failed to read from DHT sensor!");
            vTaskDelay(pdMS_TO_TICKS(3000));
            continue;  // Skip to next iteration
        }

        // ===== Update shared sensor data ======
        xSemaphoreTake(xSensorDataMutex, portMAX_DELAY);
        sharedSensorData.temperature = temperature;
        sharedSensorData.humidity = humidity;
        xSemaphoreGive(xSensorDataMutex);
        
        // ===== Update min/max values =====
        xSemaphoreTake(xSensorMinMaxMutex, portMAX_DELAY);
        if (temperature > sharedSensorMinMax.temp_max) 
            sharedSensorMinMax.temp_max = temperature;
        if (temperature < sharedSensorMinMax.temp_min) 
            sharedSensorMinMax.temp_min = temperature;
        if (humidity > sharedSensorMinMax.humi_max) 
            sharedSensorMinMax.humi_max = humidity;
        if (humidity < sharedSensorMinMax.humi_min) 
            sharedSensorMinMax.humi_min = humidity;
        
        SensorMinMax localMinMax = sharedSensorMinMax;
        xSemaphoreGive(xSensorMinMaxMutex);

        // ===== Determine and signal display state =====
        DisplayState newState = determineDisplayState(temperature, humidity);
        
        if (newState != previousState) {
            currentDisplayState = newState;
            xSemaphoreGive(xDataReadySemaphore);  // Signal other tasks
            
            SERIAL_PRINT("📊 Display State: ");
            SERIAL_PRINTLN(newState == DISPLAY_STATE_NORMAL ? "NORMAL" :
                          newState == DISPLAY_STATE_WARNING ? "WARNING" : "CRITICAL");
            
            previousState = newState;
        }

        // ===== Display based on state =====
        switch (newState) {
            case DISPLAY_STATE_NORMAL:
                displayNormalState(temperature, humidity);
                break;
            case DISPLAY_STATE_WARNING:
                displayWarningState(temperature, humidity);
                break;
            case DISPLAY_STATE_CRITICAL:
                displayCriticalState(temperature, humidity, &localMinMax);
                break;
        }

        // Print to Serial Monitor and Web Console
        char tempBuffer[128];
        sprintf(tempBuffer, "🌡️  Temp: %.1f°C  💧 Humi: %.1f%%", temperature, humidity);
        WS_LOG(String(tempBuffer));
        
        sprintf(tempBuffer, "📈 Min/Max → T: %.1f/%.1f  H: %.1f/%.1f", 
                localMinMax.temp_min, localMinMax.temp_max,
                localMinMax.humi_min, localMinMax.humi_max);
        WS_LOG(String(tempBuffer));
        WS_LOG("");  // Empty line
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}