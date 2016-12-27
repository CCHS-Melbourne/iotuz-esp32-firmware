#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_event_loop.h"

#include "mqtt.h"
#include "mqttservice.h"

static const char *TAG = "mqttservice";

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

void init_mqtt_service() {

    uint8_t sta_mac[6];
    esp_efuse_read_mac(sta_mac);

    ESP_LOGI(TAG, "sta_mac=%x:%x:%x:%x:%x:%x\n", sta_mac[0],sta_mac[1],sta_mac[2],sta_mac[3],sta_mac[4],sta_mac[5]);

    // build the client id using last 3 octets of the mac address
    char buf[32];
    sprintf(buf, "esp32-%x%x%x", sta_mac[3],sta_mac[4],sta_mac[5]);
    strcpy(settings.client_id,buf);

    mqtt_start(&settings);
}
