 #include "ioextender.h"

#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <Wire.h>
#include "pcf8574.h"

#define SDAPIN (GPIO_NUM_21)
#define SCLPIN (GPIO_NUM_22)

#define IOEXTENDER_ADDR 0x20

static const char *TAG = "ioextender";

static void i2c_scan_task(void *arg);
static void pcf8574_check_task(void *arg);
static bool isvalueinarray(int val, int *arr, int size);

static int add_arr[] = {0x1a, 0x20, 0x53, 0x77};

PCF8574 PCF_38(IOEXTENDER_ADDR);

void ioextender_init() {

  ESP_LOGI(TAG, "Setup I2C with SDA=%d, CLK=%d", SDAPIN, SCLPIN);
  Wire.begin(SDAPIN, SCLPIN);

  PCF_38.write8(255);

  xTaskCreate(i2c_scan_task, "i2c_scan_task", 4096, NULL, 1, NULL);
  xTaskCreate(pcf8574_check_task, "i2c_scan_task", 4096, NULL, 1, NULL);
}

static void i2c_scan_task(void *pvParameter)
{
  ESP_LOGI(TAG, "i2c scan task running");

  while(1)
  {
    int address;
    int foundCount = 0;

    for (address=1; address<127; address++) {
      Wire.beginTransmission(address);
      uint8_t error = Wire.endTransmission();
      if (error == 0) {
        foundCount++;
        //ESP_LOGI(TAG, "Found device at 0x%.2x", address);

        if (!isvalueinarray(address, add_arr, 4)) {
          ESP_LOGE(TAG, "Found unknown i2c device 0x%.2x", address);
        }
      }
    }

    ESP_LOGI(TAG, "Found %d I2C devices by scanning.", foundCount);

    vTaskDelay(15000 / portTICK_PERIOD_MS);
  }
}

static void pcf8574_check_task(void *pvParameter)
{
  ESP_LOGI(TAG, "pcf8574 task running");

  uint8_t value;

  while(1)
  {
    value = PCF_38.read8();

    ESP_LOGI(TAG, "Read ioextender 0x%.2x", value);

    ioextender_write(IOEXT_A_BTN, 1);
    ioextender_write(IOEXT_B_BTN, 1);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

}

void ioextender_write(uint8_t pin, uint8_t value)
{
  PCF_38.write(pin, value);
}

uint8_t ioextender_read(uint8_t pin) {
  return PCF_38.read(pin);
}

static bool isvalueinarray(int val, int *arr, int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i] == val)
            return true;
    }
    return false;
}
