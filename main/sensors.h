#pragma once

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef enum {
  SENS_TEMPERATURE,
  SENS_ALTITUDE,
  SENS_BAROMETRIC,
  SENS_ACCEL_X,
  SENS_ACCEL_Y,
  SENS_ACCEL_Z,

  SENS_MAX
} tuz_sensor_t;

/* Call to initialise sensors */
void sensors_init();

/* Call to return latest reading for a sensor */
float sensor_get(tuz_sensor_t sensor);

/* Return the name of a sensor */
const char *sensor_name(tuz_sensor_t sensor);

/* Wrapper structure of subscribing to new sensor values */
typedef struct {
  tuz_sensor_t sensor;
  float value;
} sensor_reading_t;

/* Call and pass in a queue reference to receive sensor_reading_t
   values as they are read from hardware.
*/
bool sensors_subscribe(QueueHandle_t queue);


