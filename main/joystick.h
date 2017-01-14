# pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "freertos/FreeRTOS.h"

#include <stdbool.h>
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define JOYSTICK_PIN_X 34
#define JOYSTICK_PIN_Y 39
#define JOYSTICK_PIN_SW 0

#define JOYSTICK_SAMPLE_MILLIS 50

#define GPIO_INPUT_XY_SEL  ((1<<JOYSTICK_PIN_X) | (1<<JOYSTICK_PIN_Y))
#define GPIO_INPUT_SW_SEL  (1<<JOYSTICK_PIN_SW)

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
bool joystick_subscribe(QueueHandle_t queue);

#ifdef __cplusplus
}
#endif
