#include "sensors.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

#include "esp_log.h"

#define SDAPIN (GPIO_NUM_21)
#define SCLPIN (GPIO_NUM_22)

static float readings[SENS_MAX-1];
static QueueHandle_t *subscriptions;
static size_t num_subscriptions;
static SemaphoreHandle_t sensor_mutex;

static const char *TAG = "sensors";

static void sensor_task(void *arg);

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

void sensors_init()
{
  ESP_LOGI(TAG, "I2C scanning with SDA=%d, CLK=%d", SDAPIN, SCLPIN);
  Wire.begin(SDAPIN, SCLPIN);

  int address;
  int foundCount = 0;
  for (address=1; address<127; address++) {
      Wire.beginTransmission(address);
      uint8_t error = Wire.endTransmission();
      if (error == 0) {
          foundCount++;
          ESP_LOGI(TAG, "Found device at 0x%.2x", address);
      }
  }
  ESP_LOGI(TAG, "Found %d I2C devices by scanning.", foundCount);


  if(!accel.begin()) {
    ESP_LOGE(TAG, "no ADXL345 detected.");
  } else {
    accel.setRange(ADXL345_RANGE_16_G);
  }

  sensor_mutex = xSemaphoreCreateMutex();
  xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 1, NULL);
}

/* Internal function to update a cached sensor reading */
static void sensor_set(tuz_sensor_t sensor, float value)
{
  if (sensor >= SENS_MAX) {
	return;
  }
  /* Update our cache copy of the reading */
  xSemaphoreTake(sensor_mutex, portMAX_DELAY);
  readings[sensor] = value;
  xSemaphoreGive(sensor_mutex);

  /* Try to push a reading to all subscribed tasks */
  sensor_reading_t reading = {
	.sensor = sensor,
	.value = value,
  };
  for (int i = 0; i < num_subscriptions; i++) {
	/* TODO: we assume that queueing the reading succeeds.
	   if queue is full, we may want to pop from the front
	   of the queue and then push again so the stale values
	   are always recent.
	*/
	xQueueSendToBack(subscriptions[i], &reading, 0);
  }
}

float sensor_get(tuz_sensor_t sensor)
{
  float result;
  if (sensor >= SENS_MAX) {
	return 0.0;
  }
  /* note: using 32-bit floats, mutex is not strictly
	 necessary here as reading is atomic. */
  xSemaphoreTake(sensor_mutex, portMAX_DELAY);
  result = readings[sensor];
  xSemaphoreGive(sensor_mutex);

  return result;
}

bool sensors_subscribe(QueueHandle_t queue)
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

static void sensor_task(void *arg)
{
  ESP_LOGI(TAG, "sensor task running");
  while (1) {
	vTaskDelay(10000 / portTICK_PERIOD_MS);

  sensors_event_t event; 
  bool accel_success = accel.getEvent(&event);

	for (int i = 0; i < SENS_MAX; i++) {
	  float value;

	  /* TODO: actually take readings here */
	  switch((tuz_sensor_t)i) {
	  case SENS_TEMPERATURE:
		value = sensor_get(SENS_TEMPERATURE) + 1.0;
		break;
	  case SENS_ALTITUDE:
		value = sensor_get(SENS_ALTITUDE) - 1.0;
		break;
	  case SENS_BAROMETRIC:
		value = 3.0;
		break;
    case SENS_ACCEL_X:
    if (accel_success){
      value = event.acceleration.x;
    } else {
      continue;
    }
    break;
    case SENS_ACCEL_Y:
    if (accel_success){
      value = event.acceleration.y;
    } else {
      continue;
    }
    break;
    case SENS_ACCEL_Z:
    if (accel_success){
      value = event.acceleration.z;
    } else {
      continue;
    }
    break;
	  default:
		ESP_LOGE(TAG, "invalid tuz_sensor_t value %d", i);
		continue;
	  }

 	  sensor_set((tuz_sensor_t)i, value);
	}
  }

}

const char *sensor_name(tuz_sensor_t sensor) {
  switch(sensor) {
  case SENS_TEMPERATURE:
	return "temperature";
  case SENS_ALTITUDE:
	return "altitude";
  case SENS_BAROMETRIC:
	return "barometric";
  case SENS_ACCEL_X:
  return "accelerometer_x";
  case SENS_ACCEL_Y:
  return "accelerometer_y";
  case SENS_ACCEL_Z:
  return "accelerometer_z";
  default:
	return "unknownsensor";
  }
}

