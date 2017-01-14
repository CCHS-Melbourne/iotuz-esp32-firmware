#include "joystick.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/adc.h"

static const char *TAG = "joystick";

static QueueHandle_t *subscriptions;
static size_t num_subscriptions;

static SemaphoreHandle_t interrupt_sem;

static void joystick_check_task(void *pvParameter);
static void PCFInterrupt();

void joystick_initialize()
{
  gpio_config_t io_conf;
  //disable interrupt
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  //set as output mode        
  io_conf.mode = GPIO_MODE_INPUT;
  //bit mask of the pins that you want to set,e.g.GPIO34/39
  io_conf.pin_bit_mask = GPIO_INPUT_XY_SEL;
  //disable pull-down mode
  io_conf.pull_down_en = 0;
  //disable pull-up mode
  io_conf.pull_up_en = 1;
  //configure GPIO with the given settings
  gpio_config(&io_conf);

  //interrupt of falling edge
  io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
  //bit mask of the pins, use GPIO0 here
  io_conf.pin_bit_mask = GPIO_INPUT_SW_SEL;

  gpio_config(&io_conf);

  xTaskCreatePinnedToCore(joystick_check_task, "joystick_check_task", 4096, NULL, 1, NULL, 1);
}

bool joystick_subscribe(QueueHandle_t queue)
{
  void *new_subscriptions = realloc(subscriptions, (num_subscriptions + 1) * sizeof(QueueHandle_t));
  if (!new_subscriptions) {
	ESP_LOGE(TAG, "Failed to allocate new subscription #%d", (num_subscriptions+1));
	return false;
  }

  num_subscriptions++;
  subscriptions = (QueueHandle_t *)new_subscriptions;
  subscriptions[num_subscriptions-1] = queue;
  return true;
}

static void joystick_check_task(void *pvParameter)
{

  joystick_check_s joystick = {0, 0, 0, "joystick"};

  while (1) {

    if ((xTaskGetTickCount() * portTICK_PERIOD_MS) - joystick.previous_millis >= JOYSTICK_SAMPLE_MILLIS) {

      // TODO READ
      adc1_config_width(ADC_WIDTH_12Bit);
      adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_0db);
      int val=adc1_get_voltage(ADC1_CHANNEL_0);

    }

    vTaskDelay(20 / portTICK_PERIOD_MS);      
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
