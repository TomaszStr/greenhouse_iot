#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"

#include "nvs_flash.h"
#include "driver/gpio.h"


#define LED_GPIO GPIO_NUM_2
#define BLINK_DELAY_MS 500

#define WIFI_SSID_MAX_LENGTH 64
#define WIFI_PASSWORD_MAX_LEN 64

char wifi_ssid_cpy[WIFI_SSID_MAX_LENGTH];
char wifi_password_cpy[WIFI_PASSWORD_MAX_LEN];

#define MAX_RETRIES 10

static const char *TAG = "WIFI";

static bool wifi_initialized = false;

static bool connection_failed = false;

static bool connected = false;

static bool has_ip = false;
static int retry_count = 0;

TaskHandle_t blink_task_handle = NULL;

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
    if(event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "WIFI CONNECTING....\n"); // trying to connect
    }
    else if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(TAG, "WiFi CONNECTED\n"); // connection established
        connected = true;
        retry_count = 0;
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        connected = false;
        has_ip = false;
         if (retry_count < MAX_RETRIES) {
            esp_wifi_connect();
            retry_count++;
            ESP_LOGI(TAG, "Retrying to connect to the Wi-Fi network...");
        } 
        else {
            connection_failed = true;
            if (blink_task_handle != NULL) {
                ESP_LOGI("WiFi", "Deleting blink task");
                vTaskDelete(blink_task_handle);  // Usuwamy zadanie
                blink_task_handle = NULL;  // Resetujemy uchwyt zadania
            }
            ESP_LOGE(TAG, "Failed to connect to Wi-Fi after %d retries", MAX_RETRIES);
        }
        // ESP_LOGI(TAG, "WiFi lost connection\n");
        // esp_wifi_connect(); // try to reconnect
        // ESP_LOGI(TAG, "Retrying to Connect...\n");
    }
    else if (event_id == IP_EVENT_STA_GOT_IP)
    {
        has_ip = true;
        ESP_LOGI(TAG, "Wifi got IP...\n\n");
    }
}

void configure_led()
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
}

void blink_led_task()
{
    int led_state = 0;

    while(1) {
        if(!connected){
            led_state = !led_state;
            gpio_set_level(LED_GPIO, led_state);
        }
        else {
            gpio_set_level(LED_GPIO, 0); // if connected turn off the diode
        }
        vTaskDelay(BLINK_DELAY_MS / portTICK_PERIOD_MS); // blink delay - 500ms
    }
}

void init_wifi_event_handler_station(){
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();//sets up wifi wifi_init_config struct with default values
    esp_wifi_init(&wifi_init_config); //wifi initialised with default wifi_initiation
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);//creating event handler register for wifi
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);//creating event handler register for ip event
}

void wifi_connection(){
    if(!wifi_initialized){
        esp_netif_init(); //network interdace initialization
        esp_event_loop_create_default(); //responsible for handling and dispatching events
        esp_netif_create_default_wifi_sta(); //sets up necessary data structs for wifi station interface
        wifi_initialized = true;
    }
    
    init_wifi_event_handler_station();

    wifi_config_t wifi_configuration = { 0 };
    strncpy((char *)wifi_configuration.sta.ssid, wifi_ssid_cpy, sizeof(wifi_configuration.sta.ssid));
    strncpy((char *)wifi_configuration.sta.password, wifi_password_cpy, sizeof(wifi_configuration.sta.password));

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    esp_wifi_start();
    
    esp_wifi_connect();

    printf( "wifi_init_sta finished. SSID:%s  password:%s\n",wifi_ssid_cpy,wifi_password_cpy);
}

bool wifi_check_connection(){
    return has_ip;
}

bool wifi_check_failure(){
    return connection_failed;
}

void init_wifi_connection(char* ssid, char* password){
    strncpy(wifi_ssid_cpy, ssid, WIFI_SSID_MAX_LENGTH);
    strncpy(wifi_password_cpy, password, WIFI_PASSWORD_MAX_LEN);
    wifi_connection();

    connection_failed = false;

    retry_count = 0;

    configure_led();
    ESP_LOGI(TAG, "Free heap size before task: %d\n", xPortGetFreeHeapSize());
    if (blink_task_handle != NULL) {
        ESP_LOGI("WiFi", "Deleting blink task");
        vTaskDelete(blink_task_handle);
        blink_task_handle = NULL;
    }
    xTaskCreate(&blink_led_task, "blink_led_task", 2048, NULL, 5, &blink_task_handle);
}

void stop_wifi_connection(){
    ESP_LOGI("WiFi", "Stopping WiFi connection...");

    esp_wifi_stop();

    esp_wifi_set_mode(WIFI_MODE_NULL);

    connected = false;
    connection_failed = false;

    retry_count = 0;
    memset(wifi_ssid_cpy, 0, WIFI_SSID_MAX_LENGTH);
    memset(wifi_password_cpy, 0, WIFI_PASSWORD_MAX_LEN);

    esp_wifi_deinit();

    ESP_LOGI(TAG, "Free heap size before task: %d\n", xPortGetFreeHeapSize());
    if (blink_task_handle != NULL) {
        ESP_LOGI("WiFi", "Deleting blink task");
        vTaskDelete(blink_task_handle);  // Usuwamy zadanie
        blink_task_handle = NULL;  // Resetujemy uchwyt zadania
    }
    ESP_LOGI("WiFi", "Reseting diode");
    gpio_set_level(LED_GPIO, 0);
    gpio_reset_pin(LED_GPIO);

    ESP_LOGI("WiFi", "WiFi connection stopped.");
}