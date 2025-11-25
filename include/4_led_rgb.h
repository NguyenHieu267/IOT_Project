#ifndef _4_LED_RGB_H_
#define _4_LED_RGB_H_

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// Configure pin D3-D4 
#define LED_PIN     6      
#define LED_COUNT   4      

void set_all_leds(int r, int g, int b); 

#endif