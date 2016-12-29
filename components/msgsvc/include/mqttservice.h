#ifndef _MQTT_SERVICE_H_
#define _MQTT_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

void init_mqtt_service();

/* Publish a sensor value via MQTT.

   Probably not the final interface.

   Due to threading limitations, needs to be called from the
   same task which called init_mqtt_service().
*/
void mqtt_publish_sensor(const char *sensor, float value);

#ifdef __cplusplus
}
#endif

#endif
