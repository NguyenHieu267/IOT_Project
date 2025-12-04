#include "global.h"           
#include "task_webserver.h"
#include "task_wifi.h"
#include <WiFi.h>
#include <ArduinoJson.h>

void main_server_task(void *pvParameters){
    // Đợi WiFi kết nối trước
    while (!WiFi.isConnected()) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    Serial.println("✅ WiFi connected, start sending data to WebServer");
    
    while (1) {
        // Chỉ gửi dữ liệu nếu webserver đang chạy và có client kết nối
        if (webserver_isrunning && ws.count() > 0) {
            // Tạo JSON document
            DynamicJsonDocument doc(256);
            
            xSemaphoreTake(xSensorDataMutex, portMAX_DELAY);
            doc["temperature"] = sharedSensorData.temperature;
            doc["humidity"] = sharedSensorData.humidity;
            xSemaphoreGive(xSensorDataMutex);
            
            doc["timestamp"] = millis();
            
            // Chuyển JSON thành string
            String jsonData;
            serializeJson(doc, jsonData);
            
            // Gửi qua WebSocket
            Webserver_sendata(jsonData);
        }
        
        // Gửi dữ liệu mỗi 5 giây
        vTaskDelay(500);
    }
}