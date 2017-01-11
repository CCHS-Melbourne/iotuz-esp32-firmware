#include "ioextender.h"
#include "esp_log.h"

static const char *TAG = "ioextender";

TwoWire testWire(1);
PCF857x pcf8574(0x20, &testWire);

static QueueHandle_t *subscriptions;
static size_t num_subscriptions;

volatile bool PCFInterruptFlag = false;

static void pcf8574_check_task(void *pvParameter);
void setup_pcf8574();
void PCFInterrupt();
bool check_button(button_check_s* button);

void ioextender_initialize() {
  xTaskCreatePinnedToCore(pcf8574_check_task, "pcf8574_check_task", 4096, NULL, 1, NULL, 1);
}

bool buttons_subscribe(QueueHandle_t queue)
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

static void pcf8574_check_task(void *pvParameter)
{

  button_check_s buttonA = {0, 0, HIGH, IOEXT_A_BTN, "ButtonA"};
  button_check_s buttonB = {0, 0, HIGH, IOEXT_B_BTN, "ButtonB"};
  button_check_s encoderButton = {0, 0, HIGH, IOEXT_ENCODER_BTN, "EncoderButton"};

  setup_pcf8574();

  while(1) {
    if(PCFInterruptFlag){
      // ESP_LOGI(TAG, "PCF Interrupt");
      check_button(&buttonA);
      check_button(&buttonB);
      check_button(&encoderButton);

      pcf8574.resetInterruptPin();
      PCFInterruptFlag = false;
    }

    vTaskDelay(IOEXT_POLL_INTERVAL_MILLIS / portTICK_PERIOD_MS);
  }
}

void setup_pcf8574() 
{

  testWire.begin(GPIO_NUM_21, GPIO_NUM_22);
  testWire.setClock(100000L);

  pcf8574.begin();
  
  pcf8574.write8(B00101111);
  
  pinMode(IOEXT_INTERRUPT_PIN, INPUT_PULLUP);
  pcf8574.resetInterruptPin();
  attachInterrupt(digitalPinToInterrupt(IOEXT_INTERRUPT_PIN), PCFInterrupt, FALLING);

}

// this is a simple lockout debounce function
bool check_button(button_check_s* button)
{

  uint8_t readState = pcf8574.read(button->pin);

  if (readState == button->state) {
    return false; // no state change
  }

  if (millis() - button->previous_millis >= IOEXT_DEBOUNCE_MILLIS) {
    // We have passed the time threshold, so a new change of state is allowed.
    button->previous_millis = millis();
    button->state = readState;

    button_reading_t reading = {
	  .label = button->label,
	  .state = stringFromState(button->state),
    };

    for (int i = 0; i < num_subscriptions; i++) {
      xQueueSendToBack(subscriptions[i], &reading, 0);
    }

    return true;
  }

  return false;
}

void PCFInterrupt() 
{
  PCFInterruptFlag = true;
}

void ioextender_write(uint8_t pin, uint8_t value) 
{
  //pcf8574.write(pin, value);
}