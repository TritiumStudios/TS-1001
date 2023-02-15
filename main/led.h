#ifndef LED_H
#define LED_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void init_led(void);

void set_led_color(int red, int green, int blue);

#endif