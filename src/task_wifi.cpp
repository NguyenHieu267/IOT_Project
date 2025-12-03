#include "task_wifi.h"

void startAP()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(String(SSID_AP), String(PASS_AP));
    Serial.print("📡 AP Mode Started!\n");
    Serial.print("SSID: ");
    Serial.println(String(SSID_AP));
    Serial.print("Password: ");
    Serial.println(String(PASS_AP));
    Serial.print("🌐 AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("✅ Connect to this WiFi and open: http://192.168.4.1");
}

void startSTA()
{
    if (WIFI_SSID.isEmpty())
    {
        vTaskDelete(NULL);
    }

    WiFi.mode(WIFI_STA);

    if (WIFI_PASS.isEmpty())
    {
        WiFi.begin(WIFI_SSID.c_str());
    }
    else
    {
        WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    }

    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);

    }
    //Give a semaphore here
    Serial.println("✅ WiFi connected. IP address: " + WiFi.localIP().toString());
    xSemaphoreGive(xBinarySemaphoreInternet);
}

bool Wifi_reconnect()
{
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
        return true;
    }
    startSTA();
    return false;
}
