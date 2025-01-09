#include "nvs_flash.h"
#include "esp_log.h"
#include "string.h"

#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/i2c.h"

#include "mqtt_client.h"

#include "esp_netif_sntp.h"
#include "time.h"
#include "esp_sntp.h"

#include "wifi_prod.h"
#include "ble_prod.h"
#include "mqtt_prod.h"

#include "bme280.h"
#include "capacitive_soil_moisture_sensor.h"
#include "diode.h"
#include "temt6000.h"

#include "cJSON.h"

static const char *TAG = "MAIN";

static volatile bool is_button_pressed = false;
static TimerHandle_t button_timer;

typedef enum {
    CONFIGURATION_INIT,
    CONFIGURATION_CONFIRMED,
    CONFIGURATION_WIFI_CONNECTED,
    // CONFIGURATION_MQTT_CONNECTED,
    RUNNING,
} greenhouse_iot_state_t;

static greenhouse_iot_state_t current_state = RUNNING;


/*
===================================================================================
    TIME
===================================================================================
*/

static int got_current_time = false;

void init_sntp() {
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);
}

void deinit_sntp() {
    esp_netif_sntp_deinit();
}

void get_current_time(){
    ESP_LOGI(TAG,"Try to get current time");
    time_t now = 0;
    struct tm timeinfo = { 0 };
    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) != ESP_OK) {
        ESP_LOGI(TAG,"Failed to update system time within 10s timeout");
        return;
    }
    else {
        time(&now);
        localtime_r(&now, &timeinfo);
        ESP_LOGI(TAG, "Updated system time: time:%llu, year:%d, month:%d, day:%d", (uint64_t)now, timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday);
        got_current_time = true;

        // deinit_sntp();
    }
}

uint64_t get_timestamp(){
    time_t now = 0;
    time(&now);
    return now;
}


/*
===================================================================================
    SENSORS & DEVICES VARIABLES
===================================================================================
*/

// SENSOR CONFIG

#define SENSOR_STATE_NORMAL     0
#define SENSOR_STATE_FORCED     1
#define SENSOR_STATE_SLEEP      2

#define MQTT_CMD_SET_SET_SENSOR_STATE           0
#define MQTT_CMD_SET_SET_SENSOR_READING_PERIOD  1
#define MQTT_CMD_SET_SENSOR_HEIGHT              2
#define MQTT_CMD_SET_TEMP_ALERT_THRESHOLD       3
#define MQTT_CMD_SET_SOIL_MOIST_ALERT_THRESHOLD 4

static int current_working_state = SENSOR_STATE_NORMAL;
static int height = 0;
static int  measurement_period_ms = 1000;
static int soil_moisture_alert_threshold = 10;
static int temperature_alert_threshold = 3;

#define SENSOR_SEMAPHORE_TIMEOUT_MS pdMS_TO_TICKS(1000)

// BUTTON
#define BUTTON_GPIO GPIO_NUM_0
#define LONG_PRESS_TIME_MS 3000

// ANALOG SENSORS
#define ADC_WIDTH_BIT ADC_WIDTH_BIT_12
#define ADC_ATTEN_DB ADC_ATTEN_DB_12
#define DEFAULT_VREF    3300

#define CAPACITIVE_SOIL_MOISTURE_SENSOR_ADC_CHANNEL     ADC1_CHANNEL_0
uint8_t soil_moisture_value;
capacitive_soil_moisture_sensor_t capacitive_soil_moisture_sensor;
static SemaphoreHandle_t capacitive_soil_moisture_sensor_mutex;


#define TEMT6000_ADC_CHANNEL                            ADC1_CHANNEL_5
uint8_t light_intensity_value;
temt6000_t temt6000;

// BME280
#define I2C_MASTER_SCL_IO           GPIO_NUM_22  // SCL pin
#define I2C_MASTER_SDA_IO           GPIO_NUM_21  // SDA pin
#define I2C_MASTER_NUM              I2C_NUM_0 // I2C bus number
#define I2C_MASTER_FREQ_HZ          1000000  /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0      /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0      /*!< I2C master doesn't need buffer */
#define FAIL -1

struct bme280_t bme280;

static SemaphoreHandle_t bme280_mutex;

uint8_t com_rslt = ERROR;
u32 v_pressure_u32 = 0;
s32 v_temperature_s32 = 0;
u32 v_humidity_u32 = 0;

s32 v_pressure_uncomp_u32 = 0;
s32 v_temperature_uncomp_s32 = 0;
s32 v_humidity_uncomp_u32 = 0;

u8 wait_time = 0;

double temp = 0;
char temperature[12];
double press = 0; // Pa -> hPa
char pressure[10];
double hum = 0;
char humidity[10];

void i2c_master_init()
{
	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = I2C_MASTER_SDA_IO,
		.scl_io_num = I2C_MASTER_SCL_IO,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = I2C_MASTER_FREQ_HZ};

	i2c_param_config(I2C_NUM_0, &i2c_config);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

s8 BME280_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
	s32 iError = 0;

	esp_err_t espRc;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, reg_addr, true);
	i2c_master_write(cmd, reg_data, cnt, true);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
	if (espRc == ESP_OK)
	{
		iError = SUCCESS;
	}
	else
	{
		iError = FAIL;
	}
	i2c_cmd_link_delete(cmd);

	return (s8)iError;
}

// BME280 I2C read function
s8 BME280_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
	s32 iError = 0;
	esp_err_t espRc;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, reg_addr, true);

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);

	if (cnt > 1)
	{
		i2c_master_read(cmd, reg_data, cnt - 1, I2C_MASTER_ACK);
	}
	i2c_master_read_byte(cmd, reg_data + cnt - 1, I2C_MASTER_NACK);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
	if (espRc == ESP_OK)
	{
		iError = SUCCESS;
	}
	else
	{
		iError = FAIL;
	}

	i2c_cmd_link_delete(cmd);

	return (s8)iError;
}

void BME280_delay_msek(u32 msek)
{
	vTaskDelay(msek / portTICK_PERIOD_MS);
}

static uint8_t start_bme280() {

    bme280.bus_write = BME280_I2C_bus_write;
    bme280.bus_read = BME280_I2C_bus_read;
    bme280.dev_addr = BME280_I2C_ADDRESS1;
    bme280.delay_msec = BME280_delay_msek;
	
	com_rslt = bme280_init(&bme280);

	com_rslt += bme280_set_oversamp_pressure(&bme280, BME280_OVERSAMP_16X);
	if (com_rslt != ESP_OK) { ESP_LOGE(TAG, "Failed to set press oversampling. Error code %d", com_rslt); return com_rslt; }

	com_rslt += bme280_set_oversamp_temperature(&bme280, BME280_OVERSAMP_4X);
	if (com_rslt != ESP_OK) { ESP_LOGE(TAG, "Failed to set temp oversampling. Error code %d", com_rslt); return com_rslt; }

	com_rslt += bme280_set_oversamp_humidity(&bme280, BME280_OVERSAMP_2X);
	if (com_rslt != ESP_OK) { ESP_LOGE(TAG, "Failed to set humidity oversampling. Error code %d", com_rslt); return com_rslt; }

	com_rslt += bme280_set_standby_durn(&bme280, BME280_STANDBY_TIME_63_MS);
	if (com_rslt != ESP_OK) { ESP_LOGE(TAG, "Failed to set standby duration. Error code %d", com_rslt); return com_rslt; }
	
	com_rslt += bme280_set_filter(&bme280, BME280_FILTER_COEFF_16);
	if (com_rslt != ESP_OK) { ESP_LOGE(TAG, "Failed to set filter. Error code %d", com_rslt); return com_rslt; }
	
	com_rslt += bme280_set_power_mode(&bme280, BME280_NORMAL_MODE);
	if (com_rslt != ESP_OK) { ESP_LOGE(TAG, "Failed to set power mode. Error code %d", com_rslt); return com_rslt; }

   	if (com_rslt == ESP_OK){ ESP_LOGI(TAG, "BME280 setup completed successfully");}

    return com_rslt;
}

void init_sensor_semaphores() {
    bme280_mutex = xSemaphoreCreateMutex();
    if (bme280_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create BME280 semaphore");
        return;
    }
    capacitive_soil_moisture_sensor_mutex = xSemaphoreCreateMutex();
    if (capacitive_soil_moisture_sensor_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create semaphore");
        return;
    }
}

// ALERT DIODES
#define TEMP_ALERT_DIODE_GPIO                   GPIO_NUM_14
diode_t temp_alert_diode;
uint8_t temp_alert_diode_status = 0;
uint32_t temp_alert_diode_check_period = 500;

#define SOIL_MOIST_ALERT_DIODE_GPIO             GPIO_NUM_12
diode_t soil_moist_alert_diode;
uint8_t soil_moist_alert_diode_status = 0;
uint32_t soil_moist_alert_diode_check_period = 5000;

/*
===================================================================================
    WIFI & BLE VARIABLES
===================================================================================
*/

static bool wifi_on = 0;
static bool ble_on = 0;
static bool configuration_mode = false;


#define WIFI_SSID_MAX_LENGTH 64
#define WIFI_PASSWORD_MAX_LEN 64
#define USER_MAX_LEN 64
#define MQTT_USERNAME_MAX_LEN 64
#define MQTT_URL_MAX_LEN 128
#define MQTT_PASSWORD_MAX_LEN 64

// Tablice do przechowywania wartości
char wifi_ssid[WIFI_SSID_MAX_LENGTH];
char wifi_password[WIFI_PASSWORD_MAX_LEN];
char user_value[USER_MAX_LEN];
char mqtt_username[MQTT_USERNAME_MAX_LEN];
char mqtt_url[MQTT_URL_MAX_LEN];
char mqtt_password[MQTT_PASSWORD_MAX_LEN];

char mqtt_measurements_topic[128];
char mqtt_commands_topic[128];

TaskHandle_t config_monitor_task_handle = NULL;

/*
===================================================================================
    WIFI & BLE CONNECTION
===================================================================================
*/

void monitor_config_process();
void start_wifi();

// Funkcja aktywująca tryb konfiguracyjny
void enter_config_mode() {
    ESP_LOGI(TAG, "Entering configuration mode...");
    configuration_mode = true;
    ble_server_init();
    xTaskCreate(&monitor_config_process, "monitor_config_process", 16384, NULL, 5, &config_monitor_task_handle);
}

void exit_config_mode(){
    ESP_LOGI(TAG, "Entering configuration mode...");
    configuration_mode = false;
    ble_server_stop();
    if(current_state == CONFIGURATION_CONFIRMED){
        stop_wifi_connection();
    }
    else if(current_state == CONFIGURATION_WIFI_CONNECTED){
        stop_mqtt_connection();
    }
    vTaskDelete(config_monitor_task_handle);
    current_state = RUNNING;
    start_wifi();
}

bool read_config_data_from_nvs(){
    nvs_handle_t nvs_handle;
    esp_err_t ret;

    // Otwórz przestrzeń nazw "config" w NVS w trybie tylko do odczytu
    ret = nvs_open("config", NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE("NVS", "Error opening NVS handle: %s", esp_err_to_name(ret));
        return false;
    }

    // Odczyt SSID
    size_t ssid_len = WIFI_SSID_MAX_LENGTH;
    if (nvs_get_str(nvs_handle, "ssid", wifi_ssid, &ssid_len) != ESP_OK) {
        ESP_LOGW("NVS", "SSID not found in NVS");
        nvs_close(nvs_handle);
        return false;
    }
    ESP_LOGI("NVS", "wifi_ssid: %s",wifi_ssid);

    // Odczyt hasła
    size_t password_len = WIFI_PASSWORD_MAX_LEN;
    if (nvs_get_str(nvs_handle, "password", wifi_password, &password_len) != ESP_OK) {
        ESP_LOGW("NVS", "Password not found in NVS");
        nvs_close(nvs_handle);
        return false;
    }
    ESP_LOGI("NVS", "wifi_password: %s",wifi_password);

    // // Odczyt użytkownika
    // size_t user_len = USER_MAX_LEN;
    // if (nvs_get_str(nvs_handle, "user", user_value, &user_len) != ESP_OK) {
    //     ESP_LOGW("NVS", "User not found in NVS");
    //     nvs_close(nvs_handle);
    //     return false;
    // }

    size_t mqtt_username_len =  MQTT_USERNAME_MAX_LEN;
    if (nvs_get_str(nvs_handle, "mqtt_username", mqtt_username, &mqtt_username_len) != ESP_OK) {
        ESP_LOGW("NVS", "MQTT username not found in NVS");
        nvs_close(nvs_handle);
        return false;
    }

    snprintf(mqtt_measurements_topic, sizeof(mqtt_measurements_topic), "devices/%s/data", mqtt_username);
    snprintf(mqtt_commands_topic, sizeof(mqtt_commands_topic), "devices/%s/commands", mqtt_username);
    ESP_LOGI("NVS", "mqtt_username: %s, measurements topic: %s, commands topic: %s",mqtt_username, mqtt_measurements_topic, mqtt_commands_topic);

    size_t mqtt_password_len =  MQTT_PASSWORD_MAX_LEN;
    if (nvs_get_str(nvs_handle, "mqtt_password", mqtt_password, &mqtt_password_len) != ESP_OK) {
        ESP_LOGW("NVS", "MQTT password not found in NVS");
        nvs_close(nvs_handle);
        return false;
    }
    ESP_LOGI("NVS", "mqtt_password: %s",mqtt_password);

    size_t mqtt_url_len =  MQTT_URL_MAX_LEN;
    if (nvs_get_str(nvs_handle, "mqtt_url", mqtt_url, &mqtt_url_len) != ESP_OK) {
        ESP_LOGW("NVS", "MQTT URL not found in NVS");
        nvs_close(nvs_handle);
        return false;
    }
    ESP_LOGI("NVS", "mqtt_url: %s",mqtt_url);

    nvs_close(nvs_handle);
    ESP_LOGI("NVS", "Config data loaded successfully.");

    return true;
}

static void IRAM_ATTR button_isr_handler(void *arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (gpio_get_level(BUTTON_GPIO) == 0) {
        xTimerStartFromISR(button_timer, &xHigherPriorityTaskWoken);
    } else {
        xTimerStopFromISR(button_timer, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void button_long_press_callback(TimerHandle_t xTimer) {
    is_button_pressed = true;
    ESP_LOGI(TAG, "Button held for 3 seconds.");
    if(current_state == RUNNING){
        ESP_LOGI(TAG, "Activating config mode...");
        stop_wifi_connection();
        enter_config_mode();
    }
    else {
        ESP_LOGI(TAG, "Exit config mode...");
        exit_config_mode();
    }

}

void configure_button_gpio() {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL);

    button_timer = xTimerCreate("button_timer", 
                                pdMS_TO_TICKS(LONG_PRESS_TIME_MS), 
                                pdFALSE, NULL, 
                                button_long_press_callback);
    
    configuration_mode = true;
    ESP_LOGI(TAG, "Button GPIO configured (GPIO %d)", BUTTON_GPIO);
}

bool check_config_data(){
    if(strlen(wifi_ssid) == 0){
        ESP_LOGE(TAG, "Empty SSID");
        return false;
    }
    if(strlen(mqtt_username) == 0){
        ESP_LOGE(TAG, "Empty MQTT username");
        return false;
    }
    if(strlen(mqtt_password) == 0){
        ESP_LOGE(TAG, "Empty MQTT password");
        return false;
    }
    if(strlen(mqtt_url) == 0){
        ESP_LOGE(TAG, "Empty MQTT URL");
        return false;
    }
    // if(strlen(user_value) == 0){
    //     ESP_LOGE(TAG, "Empty user name");
    //     return false;
    // }
    ESP_LOGI(TAG, "Configuration data correct");
    return true;
}

void start_wifi(){
    if(read_config_data_from_nvs()){
        if(check_config_data()){
            wifi_on = true;
            ESP_LOGI(TAG, "Configuration data correct - initializing WiFi connection");
            init_wifi_connection(wifi_ssid, wifi_password);
        }
        else{
            ESP_LOGI(TAG, "Configuration data incorrect - initializing WiFi connection failed");
        }
    }
    else{
        ESP_LOGI(TAG, "Configuration data not found in NVS - initializing WiFi connection failed");
    }
}

/*
===================================================================================
    MQTT
===================================================================================
*/

esp_mqtt_client_handle_t greenhouse_mqtt_client;

bool mqtt_connected = false;

bool mqtt_failure = false;

bool mqtt_started = false;

int mqtt_reconnect_count = 0;

char mqtt_json_data[256];

// char mqtt_url[128] = {0};
// char mqtt_username[64] = {0};
// char mqtt_password[64] = {0};

esp_err_t parse_mqtt_cmd_json_message(const char *message, int *command_out, int *value_out) {
    cJSON *root = cJSON_Parse(message);
    if (!root) {
        ESP_LOGE(TAG, "Error parsing JSON: %s", cJSON_GetErrorPtr());
        return ERROR;
    }

    cJSON *command = cJSON_GetObjectItem(root, "command");
    cJSON *value = cJSON_GetObjectItem(root, "value");

    if (!cJSON_IsNumber(command) || !cJSON_IsNumber(value)) {
        ESP_LOGW(TAG, "Invalid JSON structure. 'command' or 'value' missing or not a number.");
        cJSON_Delete(root);
        return ERROR;
    }

    *command_out = command->valueint;
    *value_out = value->valueint;

    ESP_LOGI(TAG, "Parsed JSON - Command: %d, Value: %d", *command_out, *value_out);

    cJSON_Delete(root);
    return SUCCESS;
}

void enter_normal_state() {

}

void enter_forced_state() {

}

void enter_sleep_state() {

}

void switch_sensor_state(int sensor_state) {
    switch (sensor_state)
    {
    case SENSOR_STATE_NORMAL:
        enter_normal_state();
        current_working_state = SENSOR_STATE_NORMAL;
        break;
    case SENSOR_STATE_FORCED:
        enter_forced_state();
        current_working_state = SENSOR_STATE_FORCED;
        break;
    case SENSOR_STATE_SLEEP:
        enter_sleep_state();
        current_working_state = SENSOR_STATE_SLEEP;
        break;
    default:
        ESP_LOGI(TAG, "Sensor state id:%d", sensor_state);
        break;
    }
}

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        mqtt_connected = true;
        mqtt_reconnect_count = 0;
        greenhouse_mqtt_client = client;
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        ESP_LOGI(TAG, "Subscribe to commands topic");
        esp_mqtt_client_subscribe_single(greenhouse_mqtt_client, mqtt_commands_topic, 2);
        // xTaskCreate(voltage_mqtt_task, "voltage_mqtt_task", 4096, NULL, 5, NULL);
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        mqtt_connected = false;
        mqtt_reconnect_count++;
        if(mqtt_reconnect_count > 10){
            mqtt_failure = true;
            ESP_LOGI(TAG, "MQTT connection failed...");
            stop_mqtt_connection();
        }
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED, retry count=%d",mqtt_reconnect_count);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        ESP_LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
        ESP_LOGI(TAG, "DATA=%.*s\r\n", event->data_len, event->data);

        char* message = strndup(event->data, event->data_len);
        
        if (!message) {
            ESP_LOGE(TAG, "Failed to allocate memory for MQTT message.");
            return;
        }

        int command_int = 0;
        int value_int = 0;
        if(parse_mqtt_cmd_json_message(message, &command_int, &value_int)){
            switch (command_int) {
                case(MQTT_CMD_SET_SET_SENSOR_STATE):
                    ESP_LOGI(TAG, "Switch sensor state to %d ms.", value_int);
                    switch_sensor_state(value_int);
                    break;

                case(MQTT_CMD_SET_SET_SENSOR_READING_PERIOD):
                    if(value_int < 5000) {
                        ESP_LOGW(TAG, "Sensor reading period too low: %d ms. Ignored.", value_int);
                        break;
                    }
                    ESP_LOGI(TAG, "Setting sensor reading period to %d ms.", value_int);
                    measurement_period_ms = value_int;
                    break;

                case(MQTT_CMD_SET_SENSOR_HEIGHT):
                    ESP_LOGI(TAG, "Setting sensor height to %d meters.", value_int);
                    bme280_set_altitude(&bme280, value_int);
                    break;
                    
                case(MQTT_CMD_SET_SOIL_MOIST_ALERT_THRESHOLD):
                    ESP_LOGI(TAG, "Setting soil moisture alert threshold to %d.", value_int);
                    soil_moisture_alert_threshold = value_int;
                    break;

                case(MQTT_CMD_SET_TEMP_ALERT_THRESHOLD):
                    ESP_LOGI(TAG, "Setting temperature alert threshold to %d°C.", value_int);
                    temperature_alert_threshold = value_int;
                    break;

                default:
                    ESP_LOGI(TAG, "Unknown MQTT command id:%d", command_int);
                    break;
            }
        }

        free(message);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            // log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            // log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            // log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

bool mqtt_check_failure(){
    return mqtt_failure;
}

bool mqtt_check_connection(){
    return mqtt_connected;
}

bool check_mqtt_started(){
    return mqtt_started;
}

void start_mqtt(){
    ESP_LOGI(TAG, "MQTT Config: URL=%s, Username=%s", mqtt_url, mqtt_username);
    mqtt_started = true;
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = mqtt_url,
        .credentials.username = mqtt_username,
        .credentials.authentication.password = mqtt_password,
    };
    ESP_LOGI(TAG, "MQTT initialize mqtt_client");
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    ESP_LOGI(TAG, "MQTT register event handler");
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    ESP_LOGI(TAG, "MQTT start client");
    esp_mqtt_client_start(client);
}

void stop_mqtt() {
    if (greenhouse_mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized!");
        return;
    }

    mqtt_started = false;

    ESP_LOGI(TAG, "Stopping MQTT client...");

    esp_mqtt_client_stop(greenhouse_mqtt_client);

    esp_mqtt_client_destroy(greenhouse_mqtt_client);

    ESP_LOGI(TAG, "MQTT client stopped and resources freed.");
}

/*
===================================================================================
    SENSORS & DEVICES
===================================================================================
*/

void configure_adc1(){
    adc1_config_width(ADC_WIDTH_BIT_12);
}

void init_sensors(){
    configure_adc1();

    ESP_LOGI(TAG, "Setup temperature alert diode");
    diode_init(&temp_alert_diode, TEMP_ALERT_DIODE_GPIO, temp_alert_diode_status, temp_alert_diode_check_period);

    ESP_LOGI(TAG, "Setup soil moisture alert diode");
    diode_init(&soil_moist_alert_diode, SOIL_MOIST_ALERT_DIODE_GPIO, soil_moist_alert_diode_status, soil_moist_alert_diode_check_period);

    ESP_LOGI(TAG, "Setup soil moisture sensor");
    capacitive_soil_moisture_sensor_init(&capacitive_soil_moisture_sensor, ADC_UNIT_1, CAPACITIVE_SOIL_MOISTURE_SENSOR_ADC_CHANNEL, ADC_ATTEN_DB, ADC_WIDTH_BIT, DEFAULT_VREF);
    capacitive_soil_moisture_sensor_set_oversampling(&capacitive_soil_moisture_sensor, CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_16X);

    ESP_LOGI(TAG, "Setup temt6000");
    temt6000_init(&temt6000, ADC_UNIT_1, TEMT6000_ADC_CHANNEL, ADC_ATTEN_DB, ADC_WIDTH_BIT, DEFAULT_VREF);
    temt6000_set_oversampling(&temt6000, TEMT6000_OVERSAMPLING_64X);

    ESP_LOGI(TAG, "Init I2C");
    i2c_master_init();

    ESP_LOGI(TAG, "Setup bme280");
    if(start_bme280() != SUCCESS){
        ESP_LOGI(TAG, "Retry BME280 setup");
        vTaskDelay(100/portTICK_PERIOD_MS);
        start_bme280();
    }
}

void reading_task(){
    esp_err_t ret = 0;
    while(1){
        if(current_state == RUNNING && wifi_check_connection() && mqtt_check_connection()){
            if(xSemaphoreTake(bme280_mutex, SENSOR_SEMAPHORE_TIMEOUT_MS) && xSemaphoreTake(capacitive_soil_moisture_sensor_mutex, SENSOR_SEMAPHORE_TIMEOUT_MS)) {
                if(!got_current_time) {
                    get_current_time();
                }
                else {
                    light_intensity_value = temt6000_read_compensated(&temt6000);
            
                    soil_moisture_value = capacitive_soil_moisture_sensor_read_compensated(&capacitive_soil_moisture_sensor);
                    
                    ret = bme280_read_uncomp_pressure_temperature_humidity(&bme280, &v_pressure_uncomp_u32, &v_temperature_uncomp_s32, &v_humidity_uncomp_u32);
                    if (ret != ESP_OK) {
                        ESP_LOGE(TAG, "Error reading temperature: %s", esp_err_to_name(ret));
                        vTaskDelay(measurement_period_ms/portTICK_PERIOD_MS);
                        continue;
                    }

                    temp = bme280_compensate_temperature_double(&bme280, v_temperature_uncomp_s32);
                    sprintf(temperature, "%.2f", temp);
                    
                    press = bme280_compensate_pressure_double(&bme280, v_pressure_uncomp_u32) / 100; // Pa -> hPa
                    sprintf(pressure, "%.2f", press);

                    hum = bme280_compensate_humidity_double(&bme280, v_humidity_uncomp_u32);
                    sprintf(humidity, "%.2f", hum);

                    time_t now = get_timestamp();
                    struct tm timeinfo = {0};
                    localtime_r(&now, &timeinfo);
                    char timestamp[32];
                    strftime(timestamp, 32, "%Y-%m-%dT%H:%M:%SZ", &timeinfo);

                    ESP_LOGI(TAG, "Timestamp: %s, Light intensity: %d%%, Soil moisture: %d%%, Pressure: %shPa, Temperature: %sC, Humidity: %s%%rH", timestamp, light_intensity_value, soil_moisture_value, pressure, temperature, humidity);


                    int length = snprintf(mqtt_json_data, sizeof(mqtt_json_data),
                        "{\"timestamp\": %lld, \"light_intensity\": %d, \"soil_moisture\": %d, \"pressure\": \"%s\", \"temperature\": \"%s\", \"humidity\": \"%s\"}",
                        (uint64_t)now, light_intensity_value, soil_moisture_value, pressure, temperature, humidity);

                    if (length < 0 || length >= sizeof(mqtt_json_data)) {
                        ESP_LOGE(TAG, "Failed to create JSON string or buffer overflow");
                        return;
                    }

                    ESP_LOGI(TAG, "Writing data to MQTT broker, topic:%s ,data: %s", mqtt_measurements_topic, mqtt_json_data);
                    esp_mqtt_client_publish(greenhouse_mqtt_client, mqtt_measurements_topic, mqtt_json_data, 0, 0, 0);
                }

                xSemaphoreGive(bme280_mutex);
                xSemaphoreGive(capacitive_soil_moisture_sensor_mutex);
            }
        }
        else if(current_state == RUNNING && wifi_check_connection() && !check_mqtt_started()){
            start_mqtt();
        }

        vTaskDelay(measurement_period_ms/portTICK_PERIOD_MS);
    }
}

void soil_moisture_monitoring_task(){
    while(1){
        if(xSemaphoreTake(capacitive_soil_moisture_sensor_mutex, SENSOR_SEMAPHORE_TIMEOUT_MS)) {
            soil_moisture_value = capacitive_soil_moisture_sensor_read_compensated(&capacitive_soil_moisture_sensor);
            ESP_LOGI(TAG, "Soil moisture monitoring, value: %d%%", soil_moisture_value);
            if(soil_moisture_value < soil_moisture_alert_threshold) {
                ESP_LOGI(TAG, "Soil moisture critically low: %d%%", soil_moisture_value);
                turn_on_diode(&soil_moist_alert_diode);
            }
            else {
                turn_off_diode(&soil_moist_alert_diode);
            }
            xSemaphoreGive(capacitive_soil_moisture_sensor_mutex);
        }

        vTaskDelay(soil_moist_alert_diode.check_period/portTICK_PERIOD_MS);
    }
}

void temperature_monitoring_task(){
    esp_err_t ret;
    uint8_t bme280_power_mode;
    while(1){
        // if(xSemaphoreTake(bme280_mutex, SENSOR_SEMAPHORE_TIMEOUT_MS)) {
            bme280_power_mode = 0;
            ret = bme280_get_power_mode(&bme280, &bme280_power_mode);
            if (ret != ESP_OK) {
                    ESP_LOGE(TAG, "Error reading bme280 power mode: %s", esp_err_to_name(ret));
                    vTaskDelay(temp_alert_diode.check_period / portTICK_PERIOD_MS);
                    continue;
            }

            if(bme280_power_mode == BME280_NORMAL_MODE) {
                esp_err_t ret = bme280_read_uncomp_temperature(&bme280, &v_temperature_uncomp_s32);
                if (ret != ESP_OK) {
                    ESP_LOGE(TAG, "Error reading temperature: %s", esp_err_to_name(ret));
                    vTaskDelay(temp_alert_diode.check_period / portTICK_PERIOD_MS);
                    continue;
                }
            }
            else if(bme280_power_mode == BME280_FORCED_MODE) {
                esp_err_t ret = bme280_get_forced_uncomp_pressure_temperature_humidity(&bme280, &v_pressure_uncomp_u32, &v_temperature_uncomp_s32, &v_humidity_uncomp_u32);
                if (ret != ESP_OK) {
                    ESP_LOGE(TAG, "Error reading temperature: %s", esp_err_to_name(ret));
                    vTaskDelay(temp_alert_diode.check_period / portTICK_PERIOD_MS);
                    continue;
                }
            }
            else {
                vTaskDelay(temp_alert_diode.check_period/portTICK_PERIOD_MS);
                continue;
            }

            temp = bme280_compensate_temperature_double(&bme280, v_temperature_uncomp_s32);
            ESP_LOGI(TAG, "Temperature monitoring, value: %f%%", temp);
            if(temp < temperature_alert_threshold){
                ESP_LOGI(TAG, "Temperature critically low: %f%%", temp);
                turn_on_diode(&temp_alert_diode);
            }
            else{
                turn_off_diode(&temp_alert_diode);
            }

            // xSemaphoreGive(bme280_mutex);
        // }
        vTaskDelay(temp_alert_diode.check_period/portTICK_PERIOD_MS);
    }
}

/*
===================================================================================
    SWITCH DEVICE STATES
===================================================================================
*/

void monitor_config_process(){
    wifi_on=false;
    ble_on = true;
    bool no_stop = true;
    current_state = CONFIGURATION_INIT;
    while(no_stop) {
        // // CONFIG DONE -> TURN ON WiFI
        // if(!wifi_on && check_ble_config_done()){
        //     start_wifi();
        // }
        // else if(wifi_on && check_failure() && check_ble_config_done()){
        //     notify_ble_config_failure();
        //     stop_wifi_connection();
        //     wifi_on = false;
        
        // }
        // else if(wifi_on && wifi_check_connection() && check_ble_config_done()){
        //     notify_ble_config_success();
        //     ble_server_stop();
        //     no_stop = false;
        // }

        if(current_state == CONFIGURATION_INIT && check_ble_config_done()){
            ESP_LOGI(TAG, "After config confirmed - Enter configuration-confirmed state...");
            current_state = CONFIGURATION_CONFIRMED;
            start_wifi();    
        }
        else if(current_state == CONFIGURATION_CONFIRMED && wifi_check_failure()){
            ESP_LOGI(TAG, "After WiFi failure - Return to configuration-init state...");
            current_state = CONFIGURATION_INIT;
            notify_ble_config_failure();
            stop_wifi_connection();
        }
        else if(current_state == CONFIGURATION_CONFIRMED && wifi_check_connection()){
            ESP_LOGI(TAG, "After WiFi connection success - Enter configuration-wifi-connected state...");
            current_state = CONFIGURATION_WIFI_CONNECTED;
            start_mqtt();
        }
        else if(current_state == CONFIGURATION_WIFI_CONNECTED && mqtt_check_failure()){
            ESP_LOGI(TAG, "After MQTT connection failure - Return to configuration-init state...");
            current_state = CONFIGURATION_INIT;
            notify_ble_config_failure();
            stop_mqtt(greenhouse_mqtt_client);
            stop_wifi_connection();
        }
        else if(current_state == CONFIGURATION_WIFI_CONNECTED && mqtt_check_connection()){
            ESP_LOGI(TAG, "After MQTT connection established - Enter configuration-mqtt-connecting state...");
            notify_ble_config_success();
            // get_current_time();
            ble_server_stop();
            no_stop = false;  
        }

        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    current_state = RUNNING;

    vTaskDelete(NULL);
}

void app_main() {
    
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    init_sensors();

    init_sensor_semaphores();
    if (bme280_mutex == NULL || capacitive_soil_moisture_sensor_mutex == NULL) {
        ESP_LOGE(TAG, "Semaphore initialization failed, exiting app.");
        return;
    }

    // xTaskCreate(&temperature_monitoring_task, "temperature_monitoring_task", 1024, NULL, 5, NULL);
    // xTaskCreate(&soil_moisture_monitoring_task, "soil_moisture_monitoring_task", 2048, NULL, 5, NULL);

    init_sntp();

    xTaskCreate(&reading_task, "reading_task", 4096, NULL, 5, NULL);

    current_state = RUNNING;

    start_wifi();

    configure_button_gpio();
}
