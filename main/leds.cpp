#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "led_strip.h"

#define LED_STRIP_LENGTH 2
#define LED_STRIP_RMT_INTR_NUM 19

static const char *TAG = "leds";

static struct led_color_t led_strip_buf_1[LED_STRIP_LENGTH];
static struct led_color_t led_strip_buf_2[LED_STRIP_LENGTH];

#define LED_STRIP_LENGTH 2
#define LED_STRIP_RMT_INTR_NUM 19

const uint8_t lights[360]={
  0,   0,   0,   0,   0,   1,   1,   2, 
  2,   3,   4,   5,   6,   7,   8,   9, 
 11,  12,  13,  15,  17,  18,  20,  22, 
 24,  26,  28,  30,  32,  35,  37,  39, 
 42,  44,  47,  49,  52,  55,  58,  60, 
 63,  66,  69,  72,  75,  78,  81,  85, 
 88,  91,  94,  97, 101, 104, 107, 111, 
114, 117, 121, 124, 127, 131, 134, 137, 
141, 144, 147, 150, 154, 157, 160, 163, 
167, 170, 173, 176, 179, 182, 185, 188, 
191, 194, 197, 200, 202, 205, 208, 210, 
213, 215, 217, 220, 222, 224, 226, 229, 
231, 232, 234, 236, 238, 239, 241, 242, 
244, 245, 246, 248, 249, 250, 251, 251, 
252, 253, 253, 254, 254, 255, 255, 255, 
255, 255, 255, 255, 254, 254, 253, 253, 
252, 251, 251, 250, 249, 248, 246, 245, 
244, 242, 241, 239, 238, 236, 234, 232, 
231, 229, 226, 224, 222, 220, 217, 215, 
213, 210, 208, 205, 202, 200, 197, 194, 
191, 188, 185, 182, 179, 176, 173, 170, 
167, 163, 160, 157, 154, 150, 147, 144, 
141, 137, 134, 131, 127, 124, 121, 117, 
114, 111, 107, 104, 101,  97,  94,  91, 
 88,  85,  81,  78,  75,  72,  69,  66, 
 63,  60,  58,  55,  52,  49,  47,  44, 
 42,  39,  37,  35,  32,  30,  28,  26, 
 24,  22,  20,  18,  17,  15,  13,  12, 
 11,   9,   8,   7,   6,   5,   4,   3, 
  2,   2,   1,   1,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0};

static void leds_update_task(void *pvParameter);

void leds_initialize() {
  xTaskCreatePinnedToCore(leds_update_task, "leds_update_task", 4096, NULL, 1, NULL, 1);
}

static void leds_update_task(void *pvParameter) {

  int rgb_red=0;
  int rgb_green=120;
  int rgb_blue=240;

  struct led_strip_t led_strip = {
    .rgb_led_type = RGB_LED_TYPE_APA106,
    .led_strip_length = LED_STRIP_LENGTH,
    .rmt_channel = RMT_CHANNEL_1,
    .rmt_interrupt_num = LED_STRIP_RMT_INTR_NUM,
    .gpio = GPIO_NUM_23,
    .showing_buf_1 = false,
    .led_strip_buf_1 = led_strip_buf_1,
    .led_strip_buf_2 = led_strip_buf_2,
    .access_semaphore = NULL,
  };

  led_strip.access_semaphore = xSemaphoreCreateBinary();

  bool led_init_ok = led_strip_init(&led_strip);
  assert(led_init_ok);

  struct led_color_t led_color = {
    .red = 5,
    .green = 0,
    .blue = 0,
  };

  while (true) {
    for (uint32_t index = 0; index < LED_STRIP_LENGTH; index++) {
      led_strip_set_pixel_color(&led_strip, index, &led_color);
    }
    led_strip_show(&led_strip);

    led_color.red   = lights[rgb_red];
    led_color.green = lights[rgb_green];
    led_color.blue  = lights[rgb_blue];

    rgb_red += 1;
    rgb_green += 1;
    rgb_blue += 1;

    if (rgb_red >= 360) rgb_red=0;
    if (rgb_green >= 360) rgb_green=0;
    if (rgb_blue >= 360) rgb_blue=0;

    vTaskDelay(60 / portTICK_PERIOD_MS);
  }

}
