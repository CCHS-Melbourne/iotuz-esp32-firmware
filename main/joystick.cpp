#include "joystick.h"
#include "esp_log.h"
#include <stdlib.h>

static const char *TAG = "joystick";

static QueueHandle_t *subscriptions;
static size_t num_subscriptions;

static SemaphoreHandle_t interrupt_sem;

static void joystick_check_task(void *pvParameter);
static void PCFInterrupt();

void joystick_initialize()
{
  xTaskCreatePinnedToCore(joystick_check_task, "joystick_check_task", 4096, NULL, 1, NULL, 1);
}

bool joystick_subscribe(QueueHandle_t queue)
{
  void *new_subscriptions = realloc(subscriptions, (num_subscriptions + 1) * sizeof(QueueHandle_t));
  if (!new_subscriptions) {
	ESP_LOGE(TAG, "Failed to allocate new subscription #%d", (num_subscriptions+1));
	return false;
  }

  subscriptions = (QueueHandle_t *)new_subscriptions;
  subscriptions[num_subscriptions] = queue;
  num_subscriptions++;
  return true;
}

static void joystick_check_task(void *pvParameter)
{

  joystick_check_s joystick = {0,0,0,"joystick"};

  pinMode(JOYSTICK_PIN_X, INPUT);
  pinMode(JOYSTICK_PIN_Y, INPUT);

  while (1) {

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

        for (int i = 0; i < num_subscriptions; i++) {
          xQueueSendToBack(subscriptions[i], &reading, 0);
        }

      }

    }

    vTaskDelay(50 / portTICK_PERIOD_MS);      
  }
}

static void PCFInterrupt()
{
  portBASE_TYPE higher_task_awoken;
  xSemaphoreGiveFromISR(interrupt_sem, &higher_task_awoken);
  if (higher_task_awoken) {
	portYIELD_FROM_ISR();
  }
}
