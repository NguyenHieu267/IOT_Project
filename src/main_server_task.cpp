#include "global.h"           
#include "task_webserver.h"
#include "task_wifi.h"
#include <WiFi.h>
#include <ArduinoJson.h>

void main_server_task(void *pvParameters){
    // Wait for WiFi to connect first
    while (!WiFi.isConnected()) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    SERIAL_PRINTLN("✅ WiFi connected, start sending data to WebServer");
    
    while (1) {
        // Only send data if webserver is running and there is a client connected
        if (webserver_isrunning && ws.count() > 0) {
            // Create JSON document
            DynamicJsonDocument doc(256);

            xSemaphoreTake(xSensorDataMutex, portMAX_DELAY);
            doc["temperature"] = sharedSensorData.temperature;
            doc["humidity"] = sharedSensorData.humidity;
            xSemaphoreGive(xSensorDataMutex);        

            doc["timestamp"] = millis();
            
            // Convert JSON to string
            String jsonData;
            serializeJson(doc, jsonData);
            
            // Send via WebSocket
            Webserver_sendata(jsonData);
        }
        
        // Send data every 5 seconds
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}