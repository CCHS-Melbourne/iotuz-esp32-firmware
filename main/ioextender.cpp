#include "ioextender.h"
#include "esp_log.h"

static const char *TAG = "ioextender";

static QueueHandle_t *subscriptions;
static size_t num_subscriptions;

volatile bool PCFInterruptFlag = false;

static int add_arr[] = {0x1a, 0x20, 0x53, 0x77};

static void i2c_scan_task(void *pvParameter);
static bool isvalueinarray(int val, int *arr, int size);

static void pcf8574_check_task(void *pvParameter);
void setup_pcf8574();
void PCFInterrupt();
bool check_button(PCF857x *pcf8574, button_check_s* button);

void ioextender_initialize() {
  xTaskCreatePinnedToCore(i2c_scan_task, "i2c_scan_task", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(pcf8574_check_task, "pcf8574_check_task", 4096, NULL, 1, NULL, 1);
}

bool buttons_subscribe(QueueHandle_t queue)
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

static void i2c_scan_task(void *pvParameter)
{
  ESP_LOGI(TAG, "i2c scan task running");

  TwoWire testWire(1);
  testWire.begin(GPIO_NUM_21, GPIO_NUM_22);
  testWire.setClock(100000L);

  while(1)
  {
    int address;
    int foundCount = 0;

    for (address=1; address<127; address++) {
      testWire.beginTransmission(address);
      uint8_t error = testWire.endTransmission();
      if (error == 0) {
        foundCount++;
        ESP_LOGI(TAG, "Found device at 0x%.2x", address);

        if (!isvalueinarray(address, add_arr, 4)) {
          ESP_LOGE(TAG, "Found unknown i2c device 0x%.2x", address);
        }
      }
    }

    ESP_LOGI(TAG, "Found %d I2C devices by scanning.", foundCount);

    vTaskDelay(30000 / portTICK_PERIOD_MS);
  }
}

static void pcf8574_check_task(void *pvParameter)
{
  TwoWire testWire(1);
  testWire.begin(GPIO_NUM_21, GPIO_NUM_22);
  testWire.setClock(100000L);

  button_check_s buttonA = {0, 0, HIGH, IOEXT_A_BTN, "ButtonA"};
  button_check_s buttonB = {0, 0, HIGH, IOEXT_B_BTN, "ButtonB"};
  button_check_s encoderButton = {0, 0, HIGH, IOEXT_ENCODER_BTN, "EncoderButton"};

  PCF857x pcf8574(0x20, &testWire);

  pcf8574.begin();
  
  pcf8574.write8(B00101111);
  
  pinMode(IOEXT_INTERRUPT_PIN, INPUT_PULLUP);
  pcf8574.resetInterruptPin();
  attachInterrupt(digitalPinToInterrupt(IOEXT_INTERRUPT_PIN), PCFInterrupt, FALLING);

  while(1) {
    if(PCFInterruptFlag){
      // ESP_LOGI(TAG, "PCF Interrupt");
      check_button(&pcf8574, &buttonA);
      check_button(&pcf8574, &buttonB);
      check_button(&pcf8574, &encoderButton);

      pcf8574.resetInterruptPin();
      PCFInterruptFlag = false;
    }

    vTaskDelay(IOEXT_POLL_INTERVAL_MILLIS / portTICK_PERIOD_MS);
  }
}

// this is a simple lockout debounce function
bool check_button(PCF857x *pcf8574, button_check_s* button)
{

  uint8_t readState = pcf8574->read(button->pin);

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

static bool isvalueinarray(int val, int *arr, int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i] == val)
            return true;
    }
    return false;
}
