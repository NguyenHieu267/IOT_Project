#include "DC_motor.h"

// Global variable storing the current motor state
// false = OFF, true = ON
bool dc_motor_state = false;

// Initialize the motor control pin
void dc_motor_setup() {
	pinMode(DC_MOTOR_PIN, OUTPUT);
	digitalWrite(DC_MOTOR_PIN, LOW); // Motor is off by default
	dc_motor_state = false;
}

// Turn the motor ON or OFF
void dc_motor_set(bool on) {
	dc_motor_state = on;
	digitalWrite(DC_MOTOR_PIN, on ? HIGH : LOW); // Output HIGH = ON, LOW = OFF
}