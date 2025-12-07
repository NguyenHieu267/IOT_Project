#include "relay_control.h"
#include "temp_humi_monitor.h"

// Global variable storing the current state of Relay 1
// false = OFF, true = ON
bool relay_state = false;


// Initialize the relay control pin
void relay_setup() {
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW); // Relay is off by default
    relay_state = false;
}


// Turn the motor ON or OFF
void relay_set(bool on) {
    relay_state = on;
    digitalWrite(RELAY_PIN, on ? HIGH : LOW); // Output HIGH = ON, LOW = OFF
    
    // Keep LCD from crash 
    delay(50);  // Wait for EMI to settle
    reinit_lcd();
}