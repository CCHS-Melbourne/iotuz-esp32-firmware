#pragma once

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <Wire.h>

void ioextender_init();
uint8_t ioextender_read(uint8_t pin);
void ioextender_write(uint8_t pin, uint8_t value);

