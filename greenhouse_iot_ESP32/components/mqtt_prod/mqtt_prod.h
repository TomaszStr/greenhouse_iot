#ifndef MQTT_PROD_H
#define MQTT_PROD_H

#include <stdbool.h>
#include "mqtt_client.h"

// bool mqtt_check_connection(void);
// bool mqtt_check_failure(void);
void init_mqtt_connection(esp_mqtt_client_handle_t client, esp_event_handler_t mqtt_event_handler);
void stop_mqtt_connection(void);

#endif // MQTT_PROD_H