#include "neo_blinky.h"
#include "global.h"


void neo_blinky(void *pvParameters){

    // Initialize the LED strip object 
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();

    strip.clear();  // Set all pixels to off to start
    strip.show(); // Send the current state to the LED strip

    while(1) {
        if (isNeoManualMode) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        xSemaphoreTake(xSensorDataMutex, portMAX_DELAY);
        float humidity = sharedSensorData.humidity;
        xSemaphoreGive(xSensorDataMutex);  


        // humidity < 30: yellow LED always ON
        if (humidity < 30.0) {
            strip.setPixelColor(0, strip.Color(255, 255, 0)); // Set pixel 0 to yellow
            strip.show();
        }
        // 30 < humidity < 50: yellow LED blinking 500ms
        else if (humidity < 50.0) {
            strip.setPixelColor(0, strip.Color(255, 255, 0)); // Set pixel 0 to yellow
            strip.show();
            vTaskDelay(500);
            strip.setPixelColor(0, strip.Color(0, 0, 0)); // Turn off LED
            strip.show();
            vTaskDelay(500);
        }
        // 50 <= humidity <= 60: green LED always ON
        else if (humidity <= 60.0) {
            strip.setPixelColor(0, strip.Color(0, 255, 0)); // Set pixel 0 to green
            strip.show();
        }
        // 60 < humidity < 70: red LED blinking 500ms
        else if (humidity < 70.0) {
            strip.setPixelColor(0, strip.Color(255, 0, 0)); // Set pixel 0 to red
            strip.show();
            vTaskDelay(500);
            strip.setPixelColor(0, strip.Color(0, 0, 0)); // Turn off LED
            strip.show();
            vTaskDelay(500);
        }
        // humidity >= 70: red LED always ON
        else {
            strip.setPixelColor(0, strip.Color(255, 0, 0)); // Set pixel 0 to red
            strip.show();
        }
    }
}