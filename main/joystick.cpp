#include "joystick.h"
#include "esp_log.h"
#include <stdlib.h>

static const __attribute__((unused)) char *TAG = "joystick";

static QueueHandle_t joystick_queue;

static void joystick_check_task(void *pvParameter);

void joystick_initialize()
{
  xTaskCreatePinnedToCore(joystick_check_task, "joystick_check_task", 4096, NULL, 1, NULL, 1);
}

void joystick_subscribe(QueueHandle_t queue)
{
  joystick_queue = queue;
}

static void joystick_check_task(void *pvParameter)
{

  joystick_check_s joystick = {0,0,0,"joystick"};

  pinMode(JOYSTICK_PIN_X, INPUT);
  pinMode(JOYSTICK_PIN_Y, INPUT);

  while (1) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
    if (millis() - joystick.previous_millis >= JOYSTICK_SAMPLE_MILLIS) {

      int x_value = analogRead(JOYSTICK_PIN_X);
      int y_value = analogRead(JOYSTICK_PIN_Y);

      bool changed = (abs(joystick.last_x - x_value) > 20) || (abs(joystick.last_y - y_value) > 20);

      if (changed) {
        //ESP_LOGI(TAG, "x %d y %d", x_value, y_value);

        joystick.previous_millis = millis();
        joystick.last_x = x_value;
        joystick.last_y = y_value;

        joystick_reading_t reading = {
        .label = joystick.label,
        .x_value = x_value,
        .y_value = y_value,
        };

        QueueHandle_t queue = joystick_queue;
        if (queue) {
          xQueueSendToBack(queue, &reading, 0);
        }
      }

    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}
