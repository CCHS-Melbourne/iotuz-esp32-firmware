#include "ioextender.h"
#include "esp_log.h"

static const char *TAG = "ioextender";

static QueueHandle_t ioextender_queue;

static SemaphoreHandle_t pcf_interrupt_sem;

static int add_arr[] = {0x1a, 0x20, 0x53, 0x77};

static void i2c_scan_task(void *pvParameter);
static bool isvalueinarray(int val, int *arr, int size);

static void pcf8574_check_task(void *pvParameter);
void setup_pcf8574();
static void PCFInterrupt();
bool check_button(PCF857x *pcf8574, button_check_s* button);

void ioextender_initialize() {
  pcf_interrupt_sem = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(i2c_scan_task, "i2c_scan_task", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(pcf8574_check_task, "pcf8574_check_task", 4096, NULL, 1, NULL, 1);
}

void buttons_subscribe(QueueHandle_t queue)
{
  ioextender_queue = queue;
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
    xSemaphoreTake(pcf_interrupt_sem, portMAX_DELAY);

    // ESP_LOGI(TAG, "PCF Interrupt");
    check_button(&pcf8574, &buttonA);
    check_button(&pcf8574, &buttonB);
    check_button(&pcf8574, &encoderButton);

    pcf8574.resetInterruptPin();
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

	QueueHandle_t queue = ioextender_queue;
	if (queue) {
      xQueueSendToBack(queue, &reading, 0);
    }

    return true;
  }

  return false;
}

static void IRAM_ATTR PCFInterrupt()
{
    portBASE_TYPE higher_task_awoken = pdFALSE;
	xSemaphoreGiveFromISR(pcf_interrupt_sem, &higher_task_awoken);
	if (higher_task_awoken) {
	  portYIELD_FROM_ISR();
	}
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
