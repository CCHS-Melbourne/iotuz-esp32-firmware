# pragma once

#include "freertos/FreeRTOS.h"

#include <stdbool.h>
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "pins.h"
#include <Wire.h>

#define JOYSTICK_SAMPLE_MILLIS 50

typedef struct {
  unsigned long previous_millis;
  int last_x;
  int last_y;
  const char* label;
} joystick_check_s;

typedef struct {
  const char* label;
  int x_value;
  int y_value;
} joystick_reading_t;

void joystick_initialize();
void joystick_subscribe(QueueHandle_t queue);
