#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "mqtt.h"

/* FreeRTOS event group to signal when we are connected & ready to make a tcp connection */
static EventGroupHandle_t wifi_event_group;

static const char *TAG = "iotuz";

void connected_cb(void *self, void *params) {}

void disconnected_cb(void *self, void *params) {}

void reconnect_cb(void *self, void *params) {}

void subscribe_cb(void *self, void *params) {}

void publish_cb(void *self, void *params) {}

void data_cb(void *self, void *params) {}

mqtt_settings settings = {
    .host = CONFIG_MQTT_SERVER,
    .port = 1883,
    .client_id = "mqtt_client_id",
    .username = "user",
    .password = "pass",
    .clean_session = 0,
    .keepalive = 120,
    .lwt_topic = "/lwt",
    .lwt_msg = "offline",
    .lwt_qos = 0,
    .lwt_retain = 0,
    .connected_cb = connected_cb,
    .disconnected_cb = disconnected_cb,
    .reconnect_cb = reconnect_cb,
    .subscribe_cb = subscribe_cb,
    .publish_cb = publish_cb,
    .data_cb = data_cb
};

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

void app_main(void)
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
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .bssid_set = false
        }
    };

    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );

    uint8_t sta_mac[6];
    esp_efuse_read_mac(sta_mac);

    ESP_LOGI(TAG, "sta_mac=%x:%x:%x:%x:%x:%x\n", sta_mac[0],sta_mac[1],sta_mac[2],sta_mac[3],sta_mac[4],sta_mac[5]);

    // build the client id using last 3 octets of the mac address
    char buf[32];
    sprintf(buf, "esp32-%x%x%x", sta_mac[3],sta_mac[4],sta_mac[5]);
    strcpy(settings.client_id,buf);

    mqtt_start(&settings);

    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    int level = 0;
    while (true) {
        gpio_set_level(GPIO_NUM_4, level);
        level = !level;
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}

