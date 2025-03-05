#ifndef __MY_LED__
#define __MY_LED__

#include <Arduino.h>

#define LED_BUILTIN 2

void led_init() 
{
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
}

void led_on()
{
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
}

void led_off()
{
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
}

#endif // __MY_LED__