#include <task_handler.h>
#include "task_webserver.h"
#include "DC_motor.h"
#include "relay_control.h"

void handleWebSocketMessage(String message)
{
    SERIAL_PRINTLN(message);
    StaticJsonDocument<256> doc;

    DeserializationError error = deserializeJson(doc, message);
    if (error)
    {
        SERIAL_PRINTLN("❌ JSON parse error!");
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
        } else if (gpio == 10) { // DC Motor (GPIO 10)
            if (status.equalsIgnoreCase("ON")) {
                isMotorManualMode = true;
                dc_motor_set(true);
                Serial.printf("🔆 DC Motor Manual ON\n");
            } else {
                isMotorManualMode = true;
                dc_motor_set(false);
                Serial.printf("💤 DC Motor Manual OFF\n");
            }
        } else if (gpio == 8) { // Relay (GPIO 8)
            if (status.equalsIgnoreCase("ON")) {
                isRelayManualMode = true;
                relay_set(true);
                Serial.printf("🔆 Relay Manual ON\n");
            } else {
                isRelayManualMode = true;
                relay_set(false);
                Serial.printf("💤 Relay Manual OFF\n");
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

        SERIAL_PRINTLN("📥 Collect Data from WebSocket:");
        SERIAL_PRINTLN("SSID: " + WIFI_SSID);
        SERIAL_PRINTLN("PASS: " + WIFI_PASS);
        SERIAL_PRINTLN("TOKEN: " + CORE_IOT_TOKEN);
        SERIAL_PRINTLN("SERVER: " + CORE_IOT_SERVER);
        SERIAL_PRINTLN("PORT: " + CORE_IOT_PORT);

        // 👉 Call function to save config
        Save_info_File(WIFI_SSID, WIFI_PASS, CORE_IOT_TOKEN, CORE_IOT_SERVER, CORE_IOT_PORT);

        // Reply to client with SSID info
        StaticJsonDocument<256> responseDoc;
        responseDoc["status"] = "ok";
        responseDoc["page"] = "setting_saved";
        responseDoc["ssid"] = WIFI_SSID;
        String msg;
        serializeJson(responseDoc, msg);
        ws.textAll(msg);
    }
    else if (doc["page"] == "reset")
    {
        SERIAL_PRINTLN("🔄 Received WiFi reset command...");
        String action = doc["value"]["action"].as<String>();
        
        if (action.equalsIgnoreCase("reset_wifi"))
        {
            SERIAL_PRINTLN("🗑️ Delete WiFi config, return to AP mode...");
            Delete_info_File();  // Delete config file and restart
        }
    }
}
