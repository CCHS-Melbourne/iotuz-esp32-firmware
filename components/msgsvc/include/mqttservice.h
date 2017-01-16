#ifndef _MQTT_SERVICE_H_
#define _MQTT_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

void init_mqtt_service();

/* Publish a value via MQTT.

   Probably not the final interface.

   Due to threading limitations, needs to be called from the
   same task which called init_mqtt_service().
*/
void mqtt_publish_float(const char *sensor, const char *prefix, float value);
void mqtt_publish_string(const char *sensor, const char *prefix, const char *state);
void mqtt_publish_int(const char *sensor, const char *prefix, int value);

#ifdef __cplusplus
}
#endif

#endif
