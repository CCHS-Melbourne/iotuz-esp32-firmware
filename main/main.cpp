#include "wifi.h"
#include "ioextender.h"
#include "sensors.h"
#include "rotaryencoder.h"
#include "mqttservice.h"
#include "iotuz_graphics.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

static const char *TAG = "iotuz";

static void send_sensors_task(void *pvParameter);
static void send_buttons_task(void *pvParameter);
static void send_rotaryencoder_task(void *pvParameter);

extern "C" void app_main()
{

// Initialize sub-systems in orders of dependency ...

    wifi_initialize();    
    init_mqtt_service();
    ioextender_initialize();
    rotaryencoder_initialize();
    iotuz_graphics_initialize();
    sensors_init();

    xTaskCreatePinnedToCore(send_sensors_task, "send_sensors_task", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(send_buttons_task, "send_buttons_task", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(send_rotaryencoder_task, "send_rotaryencoder_task", 4096, NULL, 1, NULL, 1);
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
          mqtt_publish_sensor(name, reading.value);
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
            mqtt_publish_button(reading.label, reading.state);
        }
    }
}

static void send_rotaryencoder_task(void *pvParameter) {

// Subscribe to button values ...

    QueueHandle_t rotaryencoder = xQueueCreate(10, sizeof(button_reading_t));

    if (! rotaryencoder_subscribe(rotaryencoder)) {
        ESP_LOGE(TAG, "Failed to subscribe to button readings :(");
    }

    while (1) {
        rotaryencoder_reading_t reading;
        if (xQueueReceive(rotaryencoder, &reading, 6000 / portTICK_PERIOD_MS)) {
            ESP_LOGI(TAG, "%s state %d",
                    reading.label,
                    reading.value);
            mqtt_publish_rotaryencoder(reading.label, reading.value);
        }
    }
}