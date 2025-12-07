#include "coreiot.h"
#include "global.h"

// ----------- CONFIGURE THESE! -----------
const char* coreIOT_Server = "app.coreiot.io";        // CORE IOT Server
const char* coreIOT_Token = "g8antxzs2o39jyb8xtgx";   // Device Access Token (DHT20)
const int   mqttPort = 1883;
// ----------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    SERIAL_PRINT("Attempting MQTT connection...");
    SERIAL_PRINT(coreIOT_Server);
    SERIAL_PRINT("...");
    // Attempt to connect (username=token, password=empty)
    //if (client.connect("ESP32Client", coreIOT_Token, NULL)) {
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), coreIOT_Token, NULL)) {
        
      SERIAL_PRINTLN("connected to CoreIOT Server!");
      client.subscribe("v1/devices/me/rpc/request/+");
      SERIAL_PRINTLN("Subscribed to v1/devices/me/rpc/request/+");

    } else {
      SERIAL_PRINT("failed, rc=");
      SERIAL_PRINT(client.state());
      SERIAL_PRINTLN(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  SERIAL_PRINT("Message arrived [");
  SERIAL_PRINT(topic);
  SERIAL_PRINTLN("] ");

  // Allocate a temporary buffer for the message
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  SERIAL_PRINT("Payload: ");
  SERIAL_PRINTLN(message);

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    SERIAL_PRINT("deserializeJson() failed: ");
    SERIAL_PRINTLN(error.c_str());
    return;
  }

  const char* method = doc["method"];
  if (strcmp(method, "setStateLED") == 0) {
    // Check params type (could be boolean, int, or string according to your RPC)
    // Example: {"method": "setValueLED", "params": "ON"}
    const char* params = doc["params"];

    if (strcmp(params, "ON") == 0) {
      SERIAL_PRINTLN("Device turned ON.");
      //TODO

    } else {   
      SERIAL_PRINTLN("Device turned OFF.");
      //TODO

    }
  } else {
    SERIAL_PRINT("Unknown method: ");
    SERIAL_PRINTLN(method);
  }
}


void setup_coreiot(){

  //Serial.print("Connecting to WiFi...");
  //WiFi.begin(wifi_ssid, wifi_password);
  //while (WiFi.status() != WL_CONNECTED) {
  
  // while (isWifiConnected == false) {
  //   delay(500);
  //   Serial.print(".");
  // }

  while(1){
    if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY)) {
      break;
    }
    delay(500);
    SERIAL_PRINT(".");
  }


  SERIAL_PRINTLN(" Connected!");

  client.setServer(CORE_IOT_SERVER.c_str(), CORE_IOT_PORT.toInt());
  client.setCallback(callback);

}

void coreiot_task(void *pvParameters){

    setup_coreiot();

    while(1){

        if (!client.connected()) {
            reconnect();
        }
        client.loop();

        // Read sensor data 
        xSemaphoreTake(xSensorDataMutex, portMAX_DELAY);
        float temp = sharedSensorData.temperature;
        float humi = sharedSensorData.humidity;
        xSemaphoreGive(xSensorDataMutex);

        // Sample payload, publish to 'v1/devices/me/telemetry'
        String payload = "{\"temperature\":" + String(temp) +  ",\"humidity\":" + String(humi) + "}";
        
        client.publish("esp/telemetry", payload.c_str());


        
        SERIAL_PRINTLN("Published payload: " + payload);
        vTaskDelay(10000);  // Publish every 10 seconds
    }
}