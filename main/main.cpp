#include "wifi.h"
#include "ioextender.h"
#include "sensors.h"
#include "joystick.h"
#include "rotaryencoder.h"
#include "mqttservice.h"
#include "iotuz_graphics.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "leds.h"

static const char *TAG = "iotuz";

static void send_telemetry_task(void *pvParameter);
static void send_sensors_task(void *pvParameter);
static void send_buttons_task(void *pvParameter);
static void send_rotaryencoder_task(void *pvParameter);
static void send_joystick_task(void *pvParameter);

extern "C" void app_main()
{

// Initialize sub-systems in orders of dependency ...
  wifi_initialize();    
  init_mqtt_service();
  ioextender_initialize();
  rotaryencoder_initialize();
  iotuz_graphics_initialize();
  sensors_initialize();
  joystick_initialize();
  leds_initialize();

  xTaskCreatePinnedToCore(send_telemetry_task, "send_telemetry_task", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(send_sensors_task, "send_sensors_task", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(send_buttons_task, "send_buttons_task", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(send_rotaryencoder_task, "send_rotaryencoder_task", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(send_joystick_task, "send_joystick_task", 4096, NULL, 1, NULL, 1);
}

static void send_telemetry_task(void *pvParameter)
{
  uint32_t value;

  while (1) {
    value = esp_get_free_heap_size();

    mqtt_publish_int("free_heap_size", "esp", value);

    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

static void send_sensors_task(void *pvParameter) {

// Subscribe to sensor values ...

  QueueHandle_t sensors = xQueueCreate(10, sizeof(sensor_reading_t));

  if (! sensors_subscribe(sensors)) {
    ESP_LOGE(TAG, "Failed to subscribe to sensor readings :(");
  }

  while (1) {
    sensor_reading_t reading;
    if (xQueueReceive(sensors, &reading, 60000 / portTICK_PERIOD_MS)) {
      const char *name = sensor_name(reading.sensor);
      ESP_LOGI(TAG, "sensor %s(%d) reading %.2f",
                name,
                reading.sensor,
                reading.value);
      mqtt_publish_float(name, "sensor", reading.value);
    }
  }
}

static void send_buttons_task(void *pvParameter) {

// Subscribe to button values ...

  QueueHandle_t buttons = xQueueCreate(10, sizeof(button_reading_t));

  if (! buttons_subscribe(buttons)) {
    ESP_LOGE(TAG, "Failed to subscribe to button readings :(");
  }

  while (1) {
    button_reading_t reading;
    if (xQueueReceive(buttons, &reading, 6000 / portTICK_PERIOD_MS)) {
        ESP_LOGI(TAG, "%s state %s",
                reading.label,
                reading.state);
        mqtt_publish_string(reading.label, "button", reading.state);
    }
  }
}

static void send_rotaryencoder_task(void *pvParameter) {

// Subscribe to rotary encoder values ...

  QueueHandle_t rotaryencoder = xQueueCreate(10, sizeof(rotaryencoder_reading_t));

  if (! rotaryencoder_subscribe(rotaryencoder)) {
    ESP_LOGE(TAG, "Failed to subscribe to button readings :(");
  }

  while (1) {
    rotaryencoder_reading_t reading;
    if (xQueueReceive(rotaryencoder, &reading, 6000 / portTICK_PERIOD_MS)) {
        ESP_LOGI(TAG, "%s reading %d",
                reading.label,
                reading.value);
        mqtt_publish_int(reading.label, "encoder", reading.value);
    }
  }
}

static void send_joystick_task(void *pvParameter) {

// Subscribe to joystick values ...

  QueueHandle_t joystick = xQueueCreate(10, sizeof(joystick_reading_t));

  if (! joystick_subscribe(joystick)) {
      ESP_LOGE(TAG, "Failed to subscribe to button readings :(");
  }

  while (1) {
    joystick_reading_t reading;
    if (xQueueReceive(joystick, &reading, 6000 / portTICK_PERIOD_MS)) {
        ESP_LOGI(TAG, "%s reading x=%d y=%d",
                reading.label,
                reading.x_value,
                reading.y_value);
        mqtt_publish_int("x", "joystick", reading.x_value);
        mqtt_publish_int("y", "joystick", reading.y_value);
    }
  }
}
