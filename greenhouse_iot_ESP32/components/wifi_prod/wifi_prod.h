#ifndef WIFI_PROD_H
#define WIFI_PROD_H

#include <stdbool.h>

bool wifi_check_connection(void);
bool wifi_check_failure(void);
void init_wifi_connection(char* ssid, char* password);
void stop_wifi_connection(void);

#endif // WIFI_PROD_H