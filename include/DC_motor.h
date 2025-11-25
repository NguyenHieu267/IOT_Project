#ifndef _DC_MOTOR_H_
#define _DC_MOTOR_H_

#include <Arduino.h>

// Configure pin D7-D8 
#define DC_MOTOR_PIN 10

void dc_motor_setup();
void dc_motor_set(bool on);

#endif
