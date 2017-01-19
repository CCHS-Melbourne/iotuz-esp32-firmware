# pragma once

#include "freertos/FreeRTOS.h"

#include <stdbool.h>
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "pins.h"
#include <pcf8574_esp.h>
#include <Wire.h>

#define RENC_DEBOUNCE_MILLIS 100

typedef struct {
    unsigned long previous_millis;
    int encoder_value;
    int last_encoded;
    int last_msb;
    int last_lsb;
    const char* label;
} rotaryencoder_check_s;

typedef struct {
    const char* label;
    int value;
} rotaryencoder_reading_t;

void rotaryencoder_initialize();
void rotaryencoder_subscribe(QueueHandle_t queue);
