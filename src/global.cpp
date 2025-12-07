#include "global.h"

// Shared sensor data (replaces glob_temperature and glob_humidity)
SensorData sharedSensorData = {0.0, 0.0};

// Shared min/max sensor data 
SensorMinMax sharedSensorMinMax = {100.0, -100.0, 100.0, -100.0};

// Current display state for LCD
DisplayState currentDisplayState = DISPLAY_STATE_NORMAL;


String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER;
String CORE_IOT_PORT;


// String WIFI_SSID = "NgHao";
// String WIFI_PASS = "nguyenhao110403";
// String CORE_IOT_TOKEN = "g8antxzs2o39jyb8xtgx";
// String CORE_IOT_SERVER = "app.coreiot.io"; 
// String CORE_IOT_PORT = "1883";

String ssid = "ESP32-YOUR NETWORK HERE!!!";
String password = "12345678";
String wifi_ssid;
String wifi_password;
boolean isWifiConnected = false;

SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();
SemaphoreHandle_t xSensorDataMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t xSensorMinMaxMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t xDataReadySemaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t xSerialMutex = xSemaphoreCreateMutex();

// NeoPixel global object 
Adafruit_NeoPixel strip(1, 45, NEO_GRB + NEO_KHZ800);

// Bool flags for Manual Mode in web server
bool isLedManualMode = false;
bool isNeoManualMode = false;
bool isMotorManualMode = false;
bool isRelayManualMode = false;