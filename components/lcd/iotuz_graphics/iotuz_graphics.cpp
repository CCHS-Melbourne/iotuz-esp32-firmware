#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "ioextender.h"
#include "iotuz_graphics.h"

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

Adafruit_ILI9341 lcd =
  Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

static const char *TAG = "iotuz_graphics";

static void iotuz_graphics_task(void *pvParameter);

void iotuz_graphics_initialize(void) {
  ESP_LOGI(TAG, "Initialize LCD screen");

  ioextender_write(IOEXT_BACKLIGHT_CONTROL, BACKLIGHT_ON);

  xTaskCreatePinnedToCore(iotuz_graphics_task, "iotuz_graphics_task", 4096, NULL, 1, NULL, 1);
}

static void iotuz_graphics_task(void *pvParameter)
{
  pinMode(TFT_CS,   OUTPUT);
  pinMode(TFT_MOSI, OUTPUT);
  pinMode(TFT_DC,   OUTPUT);
  pinMode(TFT_CLK,  OUTPUT);
  pinMode(TFT_RST,  OUTPUT);
  pinMode(TFT_MISO, OUTPUT);

  digitalWrite(TFT_CS, LOW);
  lcd.begin();

  while (1)
  {

    lcd.fillScreen(ILI9341_RED);
    lcd.fillScreen(ILI9341_GREEN);
    lcd.fillScreen(ILI9341_BLUE);

    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}
