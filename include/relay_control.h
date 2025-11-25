#ifndef _RELAY_CONTROL_H_
#define _RELAY_CONTROL_H_

#include <Arduino.h>

// Configure pin D5-D6 (USB 1)
#define RELAY_PIN  8 

void relay_setup();   
void relay_set(bool on); 

#endif