#pragma once

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <Wire.h>

#define IOEXT_ACCEL_INT 0
#define IOEXT_A_BTN 1
#define IOEXT_B_BTN 2
#define IOEXT_ENCODER_BTN 3
#define IOEXT_SDCARD_SEL 4
#define IOEXT_TOUCH_INT 5
#define IOEXT_TOUCH_SEL 6
#define IOEXT_BACKLIGHT_CONTROL 7

void ioextender_init();
uint8_t ioextender_read(uint8_t pin);
void ioextender_write(uint8_t pin, uint8_t value);

