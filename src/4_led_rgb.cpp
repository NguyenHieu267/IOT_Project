#include "4_led_rgb.h"

Adafruit_NeoPixel strip_rgb(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Turn all 4 LEDs to the same color
void set_all_leds(int r, int g, int b) {
    for(int i = 0; i < LED_COUNT; i++) {
        strip_rgb.setPixelColor(i, strip_rgb.Color(r, g, b)); // Set color 
    }
    strip_rgb.show(); // Send signal to update the LEDs
}