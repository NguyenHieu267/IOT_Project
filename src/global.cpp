#include "global.h"

String WIFI_SSID = "NgHao";
String WIFI_PASS = "nguyenhao110403";
String CORE_IOT_TOKEN = "g8antxzs2o39jyb8xtgx";
String CORE_IOT_SERVER = "app.coreiot.io"; 
String CORE_IOT_PORT = "1883";

String ssid = "ESP32-YOUR NETWORK HERE!!!";
String password = "12345678";
String wifi_ssid;
String wifi_password;
boolean isWifiConnected = false;

SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();