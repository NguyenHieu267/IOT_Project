#include <task_handler.h>
#include "task_webserver.h"

void handleWebSocketMessage(String message)
{
    Serial.println(message);
    StaticJsonDocument<256> doc;

    DeserializationError error = deserializeJson(doc, message);
    if (error)
    {
        Serial.println("❌ JSON parse error!");
        return;
    }
    JsonObject value = doc["value"];
    if (doc["page"] == "device")
    {
        if (!value.containsKey("gpio") || !value.containsKey("status"))
        {
            Serial.println("⚠️ JSON missing gpio or status info");
            return;
        }

        int gpio = value["gpio"];
        String status = value["status"].as<String>();

        Serial.printf("⚙️ Control GPIO %d → %s\n", gpio, status.c_str());
        
        if (gpio == 48) { // Onboard LED
            if (status.equalsIgnoreCase("ON")) {
                isLedManualMode = false; // Return to Auto mode
                Serial.printf("🔆 GPIO %d Auto Mode (ON)\n", gpio);
            } else {
                isLedManualMode = true; // Manual Mode
                pinMode(gpio, OUTPUT);
                digitalWrite(gpio, LOW); // Force OFF
                Serial.printf("💤 GPIO %d Forced OFF\n", gpio);
            }
        } else if (gpio == 45) { // NeoPixel
            if (status.equalsIgnoreCase("ON")) {
                isNeoManualMode = false; // Return to Auto mode
                Serial.printf("🔆 NeoPixel Auto Mode (ON)\n");
            } else {
                isNeoManualMode = true; // Manual Mode
                strip.setPixelColor(0, strip.Color(0, 0, 0)); // OFF
                strip.show();
                Serial.printf("💤 NeoPixel Forced OFF\n");
            }
        } else {
            pinMode(gpio, OUTPUT);
            if (status.equalsIgnoreCase("ON"))
            {
                digitalWrite(gpio, HIGH);
                Serial.printf("🔆 GPIO %d ON\n", gpio);
            }
            else if (status.equalsIgnoreCase("OFF"))
            {
                digitalWrite(gpio, LOW);
                Serial.printf("💤 GPIO %d OFF\n", gpio);
            }
        }
    }
    else if (doc["page"] == "setting")
    {
        String WIFI_SSID = doc["value"]["ssid"].as<String>();
        String WIFI_PASS = doc["value"]["password"].as<String>();
        String CORE_IOT_TOKEN = doc["value"]["token"].as<String>();
        String CORE_IOT_SERVER = doc["value"]["server"].as<String>();
        String CORE_IOT_PORT = doc["value"]["port"].as<String>();

        Serial.println("📥 Collect Data from WebSocket:");
        Serial.println("SSID: " + WIFI_SSID);
        Serial.println("PASS: " + WIFI_PASS);
        Serial.println("TOKEN: " + CORE_IOT_TOKEN);
        Serial.println("SERVER: " + CORE_IOT_SERVER);
        Serial.println("PORT: " + CORE_IOT_PORT);

        // 👉 Call function to save config
        Save_info_File(WIFI_SSID, WIFI_PASS, CORE_IOT_TOKEN, CORE_IOT_SERVER, CORE_IOT_PORT);

        // Reply to client (optional)
        String msg = "{\"status\":\"ok\",\"page\":\"setting_saved\"}";
        ws.textAll(msg);
    }
    else if (doc["page"] == "reset")
    {
        Serial.println("🔄 Received WiFi reset command...");
        String action = doc["value"]["action"].as<String>();
        
        if (action.equalsIgnoreCase("reset_wifi"))
        {
            Serial.println("🗑️ Delete WiFi config, return to AP mode...");
            Delete_info_File();  // Delete config file and restart
        }
    }
}
