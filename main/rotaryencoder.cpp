#include "rotaryencoder.h"
#include "esp_log.h"

static const char *TAG = "ioextender";

static QueueHandle_t *subscriptions;
static size_t num_subscriptions;

static SemaphoreHandle_t rotaryencoder_interrupt_sem;

static void rotaryencoder_check_task(void *pvParameter);
void update_encoder(rotaryencoder_check_s *encoder);
static void PCFInterrupt();

void rotaryencoder_initialize() {
  rotaryencoder_interrupt_sem = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(rotaryencoder_check_task, "rotaryencoder_check_task", 4096, NULL, 1, NULL, 1);
}

bool rotaryencoder_subscribe(QueueHandle_t queue)
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

static void rotaryencoder_check_task(void *pvParameter)
{

    pinMode(RENC_PIN1, INPUT_PULLUP);
    pinMode(RENC_PIN2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(RENC_PIN1), PCFInterrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(RENC_PIN2), PCFInterrupt, FALLING);

    rotaryencoder_check_s encoder = {0,0,0,0,0,"Encoder1"};

    while(1) {
	    xSemaphoreTake(rotaryencoder_interrupt_sem, portMAX_DELAY); /* Wait for interrupt */
      update_encoder(&encoder);
    }
}

void update_encoder(rotaryencoder_check_s *encoder)
{

  int last_encoder_value = encoder->encoder_value;

  int MSB = digitalRead(RENC_PIN1);
  int LSB = digitalRead(RENC_PIN2);

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (encoder->last_encoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoder->encoder_value ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoder->encoder_value --;

  if (encoded == 0) {
    return;
  }

  if (last_encoder_value == encoder->encoder_value) {
    return;
  }

  ESP_LOGI(TAG, "encoder read #%d", encoder->encoder_value);
  ESP_LOGI(TAG, "last_encoded #%d encoded #%d", encoder->encoder_value, encoded);

  encoder->last_encoded = encoded;
  encoder->previous_millis = millis();

  rotaryencoder_reading_t reading = {
  .label = encoder->label,
  .value = encoder->encoder_value,
  };

  // NOTE: This currently publishes a lot of messages while the encoder is being operated
  // need to optimise this to publish only changed values on a timer
  for (int i = 0; i < num_subscriptions; i++) {
      xQueueSendToBack(subscriptions[i], &reading, 0);
  }
}

static void PCFInterrupt()
{
  portBASE_TYPE higher_task_awoken = pdFALSE;
  xSemaphoreGiveFromISR(rotaryencoder_interrupt_sem, &higher_task_awoken);
  if (higher_task_awoken) {
	portYIELD_FROM_ISR();
  }
}
