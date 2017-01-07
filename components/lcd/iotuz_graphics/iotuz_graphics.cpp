#include "esp_log.h"

#include "iotuz_graphics.h"

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

Adafruit_ILI9341 lcd =
  Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

static const char *TAG = "iotuz_graphics";

void iotuz_graphics_initialize(void) {
  ESP_LOGI(TAG, "Initialize LCD screen");

  pinMode(TFT_CS,   OUTPUT);
  pinMode(TFT_MOSI, OUTPUT);
  pinMode(TFT_DC,   OUTPUT);
  pinMode(TFT_CLK,  OUTPUT);
  pinMode(TFT_RST,  OUTPUT);
  pinMode(TFT_MISO, OUTPUT);

  digitalWrite(TFT_CS, LOW);
  lcd.begin();

  lcd.fillScreen(ILI9341_RED);
  lcd.fillScreen(ILI9341_GREEN);
  lcd.fillScreen(ILI9341_BLUE);
}
