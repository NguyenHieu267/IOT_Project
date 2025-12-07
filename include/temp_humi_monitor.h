#ifndef __TEMP_HUMI_MONITOR__
#define __TEMP_HUMI_MONITOR__
#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include "DHT20.h"
#include "global.h"

extern LiquidCrystal_I2C lcd;  // Exported for relay reinit

void temp_humi_monitor(void *pvParameters);
void reinit_lcd();  // Reinitialize LCD after relay switch


#endif