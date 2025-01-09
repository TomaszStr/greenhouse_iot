#ifndef BLE_SERVER_H
#define BLE_SERVER_H

void ble_server_init(void);
void ble_server_stop(void);
bool check_ble_config_done(void);

void notify_ble_wifi_begin();
void notify_ble_config_failure();
void notify_ble_config_success();
void notify_ble_mqtt_failure();

#endif