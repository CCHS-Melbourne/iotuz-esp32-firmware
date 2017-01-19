# pragma once

#include "freertos/FreeRTOS.h"

#include <stdbool.h>
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "pins.h"
#include <pcf8574_esp.h>
#include <Wire.h>

#define IOEXT_ACCEL_INT          0
#define IOEXT_A_BTN              1
#define IOEXT_B_BTN              2
#define IOEXT_ENCODER_BTN        3
#define IOEXT_SDCARD_SEL         4
#define IOEXT_TOUCH_INT          5
#define IOEXT_TOUCH_SEL          6
#define IOEXT_BACKLIGHT_CONTROL  7

#define BACKLIGHT_ON   0
#define BACKLIGHT_OFF  1

#define IOEXT_POLL_INTERVAL_MILLIS 50
#define IOEXT_DEBOUNCE_MILLIS 100

typedef struct {
    unsigned long previous_millis;
    uint16_t last_state;
    uint8_t state;
    uint8_t pin;
    const char* label;
} button_check_s;

typedef struct {
    const char* label;
    const char* state;
} button_reading_t;

static inline const char* stringFromState(uint8_t bs)
{
    static const char *strings[] = { "DOWN", "UP"};

    return strings[bs];
}

void ioextender_initialize();
void buttons_subscribe(QueueHandle_t queue);
void ioextender_write(uint8_t pin, uint8_t value);
