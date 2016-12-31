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
#include <Wire.h>

#include "sensors.h"

#define SDAPIN (GPIO_NUM_21)
#define SCLPIN (GPIO_NUM_22)

/* FreeRTOS event group to signal when we are connected & ready to make a tcp connection */
static EventGroupHandle_t wifi_event_group;
int xtender_address = 0x20;

static const char *TAG = "iotuz";

TwoWire i2cwire(0);

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

uint8_t pcf8574_write(uint8_t dt){
  i2cwire.beginTransmission(xtender_address);
  i2cwire.write(dt);

  return i2cwire.endTransmission();
}

bool PCFInterruptFlag = false;

void PCFInterrupt() {
  ESP_LOGI(TAG, "Interrupt running");
  PCFInterruptFlag = true;
}

void init_ioextender() {
    uint8_t error;

    // turn 0ff the screen
    error = pcf8574_write(B11111111);

    if (error) {
        ESP_LOGE(TAG, "io extender error: %x", error);
    }

    delay(1000);

    // turn on the screen
    error = pcf8574_write(B01111111);

    if (error) {
        ESP_LOGE(TAG, "io extender error: %x", error);
    }

    pinMode(GPIO_NUM_25, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(GPIO_NUM_25), PCFInterrupt, FALLING);

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

    ESP_LOGI(TAG, "Setup I2C with SDA=%d, CLK=%d", SDAPIN, SCLPIN);
    i2cwire.begin(SDAPIN, SCLPIN);

    ESP_LOGI(TAG, "io extender=%x", xtender_address);
    init_ioextender();

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
        ESP_LOGI(TAG, "check interrupt");
        if (PCFInterruptFlag) {
            ESP_LOGI(TAG, "PCFInterruptFlag");
            PCFInterruptFlag = false;
        }
    }
}
