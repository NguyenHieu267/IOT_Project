#include "4_led_rgb.h"

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Turn all 4 LEDs to the same color
void set_all_leds(int r, int g, int b) {
    for(int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b)); // Set color 
    }
    strip.show(); // Send signal to update the LEDs
}