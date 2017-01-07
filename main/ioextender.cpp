#include "ioextender.h"

#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <Wire.h>

#define SDAPIN (GPIO_NUM_21)
#define SCLPIN (GPIO_NUM_22)

#define IOEXTENDER_ADDR 0x20

static const char *TAG = "ioextender";

static void i2c_scan_task(void *arg);
static bool isvalueinarray(int val, int *arr, int size);

static int add_arr[] = {0x1a, 0x20, 0x53, 0x77};

void ioextender_init() {

  ESP_LOGI(TAG, "Setup I2C with SDA=%d, CLK=%d", SDAPIN, SCLPIN);
  Wire.begin(SDAPIN, SCLPIN);

  xTaskCreate(i2c_scan_task, "i2c_scan_task", 4096, NULL, 1, NULL);
}

static void i2c_scan_task(void *pvParameter)
{
  ESP_LOGI(TAG, "task running");

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

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

static bool isvalueinarray(int val, int *arr, int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i] == val)
            return true;
    }
    return false;
}
