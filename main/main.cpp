#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "mqttservice.h"
#include <Arduino.h>

#include "sensors.h"



/* FreeRTOS event group to signal when we are connected & ready to make a tcp connection */
static EventGroupHandle_t wifi_event_group;

static const char *TAG = "iotuz";


esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

extern "C" void app_main()
{

    nvs_flash_init();
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );

    // reads the wifi configuration from kconfig
    // to update this run `make menuconfig`
    wifi_config_t wifi_config = {};

    strcpy(wifi_config.sta.ssid, CONFIG_WIFI_SSID);
    strcpy(wifi_config.sta.password, CONFIG_WIFI_PASSWORD);
    wifi_config.sta.bssid_set = false;

    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );

    ESP_LOGI(TAG, "MQTT server=%s", CONFIG_MQTT_SERVER);
    init_mqtt_service();

    /* start sensor data collection */
    sensors_init();

    /* subscribe to sensor values */
    QueueHandle_t sensors = xQueueCreate(10, sizeof(sensor_reading_t));
    if (!sensors_subscribe(sensors)) {
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
