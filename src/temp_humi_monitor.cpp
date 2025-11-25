#include "temp_humi_monitor.h"
#include "global.h"


#define TEMP_HIGH 35.0
#define TEMP_LOW  15.0
#define HUMI_HIGH  80.0
#define HUMI_LOW   20.0
#define LED 13

DHT20 dht20;
LiquidCrystal_I2C lcd(33,16,2);

float temp_min = 100.0;
float temp_max = -100.0;
float humi_min = 100.0;
float humi_max = -100.0;

void temp_humi_monitor(void *pvParameters){

    Wire.begin(11, 12);
    Serial.begin(115200);
    dht20.begin();

    lcd.begin();
    lcd.backlight();

    pinMode(LED, OUTPUT);

    Serial.println("Begin");
    while (1){
        dht20.read();
        // Reading temperature in Celsius
        float temperature = dht20.getTemperature();
        // Reading humidity
        float humidity = dht20.getHumidity();

        //alert
        if (temperature > temp_max) temp_max = temperature;
        if (temperature < temp_min) temp_min = temperature;
        if (humidity    > humi_max) humi_max = humidity;
        if (humidity    < humi_min) humi_min = humidity;

        bool alert = false;
        if (temperature > TEMP_HIGH || temperature < TEMP_LOW ||
        humidity > HUMI_HIGH || humidity < HUMI_LOW) {
        alert = true;
        digitalWrite(LED, HIGH);
        } else {
        digitalWrite(LED, LOW);
        }

        lcd.clear();
        if (alert){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("HIGH TEMP: ");
            lcd.print(temp_max,1);
            vTaskDelay(2000);

            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("HIGH HUMID: ");
            lcd.print(humi_max,1);
            lcd.print("%");
            vTaskDelay(2000);

        } else if(temperature < TEMP_LOW || humidity < HUMI_LOW){  //low temp & humid
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("LOW TEMP: ");
            lcd.print(temp_min,1);
            vTaskDelay(2000);

            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("LOW HUMID: ");
            lcd.print(humi_min,1);
            lcd.print("%");
            vTaskDelay(2000);
        } else{   //normal
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Temp: ");
            lcd.print(temperature, 1);
            lcd.print(" C");

            lcd.setCursor(0,1);
            lcd.print("Humi: ");
            lcd.print(humidity, 1);
            lcd.print(" %");
            }


        // Check if any reads failed and exit early
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity =  -1;
            //return;
        }

        //Update global variables for temperature and humidity 
        xSemaphoreTake(xMutexSensorData, portMAX_DELAY);
        glob_temperature = temperature;
        glob_humidity = humidity;
        xSemaphoreGive(xMutexSensorData);

        // Print the results
        
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
        Serial.print("MAX TEMP: ");
        Serial.print(temp_max);
        Serial.print("LOWEST TEMP: ");
        Serial.print(temp_min);
        Serial.print("HIGH HUMIDITY: ");
        Serial.print(humi_max);
        Serial.print("LOWEST HUMID: ");
        Serial.println(humi_min);
        
        vTaskDelay(5000);
    }
    
}