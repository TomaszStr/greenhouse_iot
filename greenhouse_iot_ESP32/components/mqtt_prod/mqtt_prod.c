#include "mqtt_prod.h"
#include "esp_log.h"

// #include <stdio.h>
#include <stdint.h>
// #include <stddef.h>
// #include <string.h>

// #define CONFIG_BROKER_URL "mqtt://192.168.137.1:1883"
// #define CONFIG_BROKER_PORT 1883
// #define CONFIG_BROKER_IP "192.168.137.1"

char* TAG = "mqtt_prod";

// char* device_name = "device_1";
// char* mqtt_url = "";
// char* mqtt_username;
// char* mqtt_password;
// char* mqtt_measurements_topic;
// char* mqtt_commands_topic;

// bool mqtt_connected = false;

// bool mqtt_failure = false;

uint8_t mac[6];

esp_mqtt_client_handle_t mqtt_client = NULL;

void init_mqtt_connection(esp_mqtt_client_handle_t client, esp_event_handler_t mqtt_event_handler){
    mqtt_client = client;
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

void stop_mqtt_connection(void){
    if (mqtt_client != NULL) {
        esp_mqtt_client_stop(mqtt_client);
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
    }
}

// bool mqtt_check_connection(void){
//     return true;
// }
// bool mqtt_check_failure(void){
//     return false;
// }


// /* MQTT (over TCP) Example

//    This example code is in the Public Domain (or CC0 licensed, at your option.)

//    Unless required by applicable law or agreed to in writing, this
//    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//    CONDITIONS OF ANY KIND, either express or implied.
// */

// #include <stdio.h>
// #include <stdint.h>
// #include <stddef.h>
// #include <string.h>
// #include "esp_wifi.h"
// #include "esp_system.h"
// #include "nvs_flash.h"
// #include "esp_event.h"
// #include "esp_netif.h"
// #include "protocol_examples_common.h"

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/semphr.h"
// #include "freertos/queue.h"

// #include "lwip/sockets.h"
// #include "lwip/dns.h"
// #include "lwip/netdb.h"

// #include "esp_log.h"
// #include "mqtt_client.h"

// #include "time.h"
// #include "esp_sntp.h"

// #include "driver/adc.h"
// #include "esp_adc_cal.h"



// #define CONFIG_BROKER_URL "mqtt://192.168.137.1:1883"
// #define CONFIG_BROKER_PORT 1883
// #define CONFIG_BROKER_IP "192.168.137.1"

// static const char *TAG = "mqtt_example";

// static const char* device_name = "device_1";

// static bool mqtt_connected = false;

// static uint8_t mac[6];

// static int measurement_interval = 10000;

// esp_mqtt_client_handle_t mqtt_client = NULL;

// #define ADC_CHANNEL     ADC1_CHANNEL_0  // Kanał ADC1 GPIO36 (VP)
// #define DEFAULT_VREF    5000            // Wartość referencyjna napięcia w mV (można skalibrować)
// #define NO_OF_SAMPLES   64              // Liczba próbek do średniej


// static void log_error_if_nonzero(const char *message, int error_code)
// {
//     if (error_code != 0) {
//         ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
//     }
// }


// void load_MAC_address(){
//     esp_err_t ret = esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);

//     if (ret == ESP_OK) {
//         ESP_LOGI("MAC", "WiFi MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
//                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
//     } else {
//         ESP_LOGE("MAC", "Failed to get WiFi MAC address");
//     }
// }

// void voltage_mqtt_task() {
//     char payload[128];
//     char topic[64];
//     char timestamp[32];

//     init_analog_port();

//     while (true) {
//         if (mqtt_connected && mqtt_client) {
//             // Pobierz wartość z ADC
//             uint32_t voltage = read_analog();

//             // Pobierz znacznik czasu
//             get_timestamp(timestamp, sizeof(timestamp));

//             // Skonstruuj wiadomość JSON
//             snprintf(
//                 payload, sizeof(payload),
//                 "{\"device\":\"%s\",\"mac\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\"timestamp\":\"%s\",\"voltage\":%lu}",
//                 device_name,
//                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
//                 timestamp,
//                 voltage
//             );

//             // Temat wiadomości (można dostosować)
//             snprintf(topic, sizeof(topic), "/greenhouse/%s/soil_moisture", device_name);

//             // Wyślij wiadomość do brokera
//             int msg_id = esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);
//             ESP_LOGI(TAG, "Sent publish successful, msg_id=%d, payload=%s", msg_id, payload);
//         } else {
//             ESP_LOGW(TAG, "MQTT not connected, skipping publish");
//         }

//         // Czekaj na kolejną iterację
//         vTaskDelay(pdMS_TO_TICKS(measurement_interval));
//     }
// }

// /*
//  * @brief Event handler registered to receive MQTT events
//  *
//  *  This function is called by the MQTT client event loop.
//  *
//  * @param handler_args user data registered to the event.
//  * @param base Event base for the handler(always MQTT Base in this example).
//  * @param event_id The id for the received event.
//  * @param event_data The data for the event, esp_mqtt_event_handle_t.
//  */
// static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
// {
//     ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
//     esp_mqtt_event_handle_t event = event_data;
//     esp_mqtt_client_handle_t client = event->client;
//     int msg_id;
//     switch ((esp_mqtt_event_id_t)event_id) {
//     case MQTT_EVENT_CONNECTED:
//         mqtt_connected = true;
//         mqtt_client = client;
//         ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

//         xTaskCreate(voltage_mqtt_task, "voltage_mqtt_task", 4096, NULL, 5, NULL);
//         // msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
//         // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

//         // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
//         // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

//         // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
//         // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

//         // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
//         // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
//         break;
//     case MQTT_EVENT_DISCONNECTED:
//         mqtt_connected = false;
//         ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
//         break;

//     case MQTT_EVENT_SUBSCRIBED:
//         ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
//         // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
//         // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
//         break;
//     case MQTT_EVENT_UNSUBSCRIBED:
//         ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
//         break;
//     case MQTT_EVENT_PUBLISHED:
//         ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
//         break;
//     case MQTT_EVENT_DATA:
//         ESP_LOGI(TAG, "MQTT_EVENT_DATA");
//         printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
//         printf("DATA=%.*s\r\n", event->data_len, event->data);
//         break;
//     case MQTT_EVENT_ERROR:
//         ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
//         if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
//             log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
//             log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
//             log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
//             ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
//         }
//         break;
//     default:
//         ESP_LOGI(TAG, "Other event id:%d", event->event_id);
//         break;
//     }
// }

// static void mqtt_app_start(void)
// {
//     esp_mqtt_client_config_t mqtt_cfg = {
//         .broker.address.uri = CONFIG_BROKER_URL,
//         // .broker.address.port = CONFIG_BROKER_PORT,
//         // .broker.address.hostname = CONFIG_BROKER_IP
//     };

//     esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
//     /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
//     esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
//     esp_mqtt_client_start(client);
// }

// // void app_main(void)
// // {
// //     ESP_LOGI(TAG, "[APP] Startup..");
// //     ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
// //     ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

// //     esp_log_level_set("*", ESP_LOG_INFO);
// //     esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
// //     esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);
// //     esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
// //     esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
// //     esp_log_level_set("transport", ESP_LOG_VERBOSE);
// //     esp_log_level_set("outbox", ESP_LOG_VERBOSE);

// //     ESP_ERROR_CHECK(nvs_flash_init());
// //     ESP_ERROR_CHECK(esp_netif_init());
// //     ESP_ERROR_CHECK(esp_event_loop_create_default());

// //     /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
// //      * Read "Establishing Wi-Fi or Ethernet Connection" section in
// //      * examples/protocols/README.md for more information about this function.
// //      */
// //     ESP_ERROR_CHECK(example_connect());

// //     load_MAC_address();
// //     obtain_time();

// //     mqtt_app_start();
// // }