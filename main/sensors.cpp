#include "sensors.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "SparkFunBME280.h"

#include "esp_log.h"

static float readings[SENS_MAX];

static QueueHandle_t sensor_queue;
static SemaphoreHandle_t sensor_mutex;

static const char *TAG = "sensors";

static void sensor_task(void *arg);

void sensors_initialize()
{
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
  QueueHandle_t queue = sensor_queue;
  if (queue) {
	/* TODO: we assume that queueing the reading succeeds.
	   if queue is full, we may want to pop from the front
	   of the queue and then push again so the stale values
	   are always recent.
	*/
	xQueueSendToBack(queue, &reading, 0);
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

void sensors_subscribe(QueueHandle_t queue)
{
  sensor_queue = queue;
}

int loops;

static void sensor_task(void *arg)
{
  ESP_LOGI(TAG, "sensor task running");

  TwoWire i2cWire(1);

  i2cWire.begin(GPIO_NUM_21, GPIO_NUM_22);
  i2cWire.setClock(100000L);

  BME280 bme280(0x77, &i2cWire);
  sensor_mutex = xSemaphoreCreateMutex();

  ESP_LOGI(TAG, "BME280 0x%02x", bme280.begin());
  ESP_LOGI(TAG, "ID(0xD0) 0x%02x", bme280.readRegister(BME280_CHIP_ID_REG));

  ESP_LOGI(TAG, "Displaying ID, reset and ctrl regs\n");

  ESP_LOGI(TAG, "ID(0xD0): 0x%02x", bme280.readRegister(BME280_CHIP_ID_REG));
  ESP_LOGI(TAG, "Reset register(0xE0): 0x%02x", bme280.readRegister(BME280_RST_REG));
  ESP_LOGI(TAG, "ctrl_meas(0xF4): 0x%02x", bme280.readRegister(BME280_CTRL_MEAS_REG));
  ESP_LOGI(TAG, "ctrl_hum(0xF2): 0x%02x", bme280.readRegister(BME280_CTRL_HUMIDITY_REG));

  bme280.settings.runMode = 3; //Normal mode
  bme280.settings.tStandby = 0;
  bme280.settings.filter = 2;
  bme280.settings.tempOverSample = 1;
  bme280.settings.pressOverSample = 1;
  bme280.settings.humidOverSample = 1;

  loops = 0;

  while (1) {
	vTaskDelay(100 / portTICK_PERIOD_MS);

  if (loops < 100) {
    loops++;
    continue;
  }

	for (int i = 0; i < SENS_MAX; i++) {
	  float value;

	  /* TODO: actually take readings here */
	  switch((tuz_sensor_t)i) {
	  case SENS_TEMPERATURE:
		value = bme280.readTempC();
		break;
	  case SENS_HUMIDITY:
    value = bme280.readFloatHumidity();
		break;
	  case SENS_ALTITUDE:
    value = bme280.readFloatAltitudeMeters();
		break;
	  case SENS_BAROMETRIC:
		value = bme280.readFloatPressure();
		break;
	  default:
		ESP_LOGE(TAG, "invalid tuz_sensor_t value %d", i);
		continue;
	  }

 	  sensor_set((tuz_sensor_t)i, value);

    loops = 0;
	}
  }

}

const char *sensor_name(tuz_sensor_t sensor) {
  switch(sensor) {
  case SENS_TEMPERATURE:
	return "temperature";
  case SENS_HUMIDITY:
	return "humidity";
  case SENS_ALTITUDE:
	return "altitude";
  case SENS_BAROMETRIC:
	return "barometric";
  default:
	return "unknownsensor";
  }
}

