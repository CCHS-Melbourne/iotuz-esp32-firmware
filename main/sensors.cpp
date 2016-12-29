#include "sensors.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_log.h"

static float readings[SENS_MAX-1];
static QueueHandle_t *subscriptions;
static size_t num_subscriptions;
static SemaphoreHandle_t sensor_mutex;

static const char *TAG = "sensors";

static void sensor_task(void *arg);

void sensors_init()
{
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
  default:
	return "unknownsensor";
  }
}

