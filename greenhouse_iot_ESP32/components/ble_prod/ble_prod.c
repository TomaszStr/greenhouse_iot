#include <string.h>
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"

#include "nvs_flash.h"
// #include "nvs.h"

static const char *TAG = "BLE";

// Application Profile - A config
#define GATTS_SERVICE_UUID  0x00FF
#define GATTS_CHAR_UUID_SSID 0xFF01
#define GATTS_CHAR_UUID_PASSWORD 0xFF02
#define GATTS_CHAR_UUID_USER_ID 0xFF03
#define GATTS_CHAR_UUID_CONFIRM 0xFF04
#define GATTS_CHAR_UUID_NOTIFY 0xFF05

// OLD
// // Application Profile - A config
// #define GATTS_SERVICE_UUID  0x00FF
// #define GATTS_CHAR_UUID_SSID 0xFF01
// #define GATTS_CHAR_UUID_PASSWORD 0xFF02
// #define GATTS_CHAR_UUID_MQTT_USERNAME 0xFF03
// #define GATTS_CHAR_UUID_MQTT_URL 0xFF04
// #define GATTS_CHAR_UUID_MQTT_PASSWORD 0xFF05
// #define GATTS_CHAR_UUID_CONFIRM 0xFF06
// #define GATTS_CHAR_UUID_NOTIFY 0xFF07

#define GATTS_NUM_HANDLE     40

// Rozmiar danych dla charakterystyk
#define CHAR_VALUE_MAX_LEN 128
// #define CHAR_MQTT_MAX_LEN 128


#define TEST_DEVICE_NAME            "ESP_GATTS_greenhouse"
#define TEST_MANUFACTURER_DATA_LEN  17

// Number of GATT server profiles
#define PROFILE_NUM 1
#define PAIRING_PROFILE_APP_ID 0

#define PREPARE_BUF_MAX_SIZE 256

static bool ble_configuration_done = false;
static bool ble_initialized = false;
static bool ble_enabled = false;

static uint16_t wifi_ssid_handle = 0;
static uint16_t wifi_password_handle = 0;
static uint16_t user_id_handle = 0;
// static uint16_t mqtt_username_handle = 0;
// static uint16_t mqtt_url_handle = 0;
// static uint16_t mqtt_password_handle = 0;
static uint16_t confirm_handle = 0;
static uint16_t notify_handle = 0;

static int notify_conn_id = -1;
static int notify_gatts_if = -1;

typedef struct {
    uint8_t value[CHAR_VALUE_MAX_LEN];
    size_t length;
} char_value_t;

static char_value_t ssid_value = {0};
static char_value_t password_value = {0};
static char_value_t user_id_value = {0};
// static char_value_t mqtt_url_value = {0};
// static char_value_t mqtt_username_value = {0};
// static char_value_t mqtt_password_value = {0};
static char_value_t confirm_value = {0};



static void gatts_pairing_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

static uint8_t adv_config_done = 0;
#define adv_config_flag      (1 << 0)
#define scan_rsp_config_flag (1 << 1)

#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[] = {
        0x02, 0x01, 0x06,                  // Length 2, Data Type 1 (Flags), Data 1 (LE General Discoverable Mode, BR/EDR Not Supported)
        0x02, 0x0a, 0xeb,                  // Length 2, Data Type 10 (TX power level), Data 2 (-21)
        0x03, 0x03, 0xab, 0xcd,            // Length 3, Data Type 3 (Complete 16-bit Service UUIDs), Data 3 (UUID)
};
static uint8_t raw_scan_rsp_data[] = {     // Length 15, Data Type 9 (Complete Local Name), Data 1 (ESP_GATTS_DEMO)
        0x0f, 0x09, 0x45, 0x53, 0x50, 0x5f, 0x47, 0x41, 0x54, 0x54, 0x53, 0x5f, 0x44,
        0x45, 0x4d, 0x4f
};
#else

static uint8_t adv_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
    //second uuid, 32bit, [12], [13], [14], [15] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// ADVERTISEMENT DATA CONFIG
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    // .min_interval = 0x0006,
    // .max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// RESPONSE DATA CONFIG
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    //.min_interval = 0x0006,
    //.max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

#endif /* CONFIG_SET_RAW_ADV_DATA */

// Define GATT server profile struct for config
struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_attr_value_t char_value;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

// Add created profiles to GATT server, more profiles -> more gatts_profile_inst in array
/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PAIRING_PROFILE_APP_ID] = {
        .gatts_cb = gatts_pairing_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,
    }
};

// Buffer config struct
typedef struct {
    uint8_t                 *prepare_buf;
    int                     prepare_len;
    uint16_t                handle;
} prepare_type_env_t;

static prepare_type_env_t pairing_prepare_write_env;

// typedef struct {
//     char ssid[CHAR_VALUE_MAX_LEN];
//     char password[CHAR_VALUE_MAX_LEN];
//     char user[CHAR_VALUE_MAX_LEN];
//     char mqtt[CHAR_MQTT_MAX_LEN];
//     char device[CHAR_VALUE_MAX_LEN];
// } ble_data_t;

static void configure_characteristics() {
    esp_attr_value_t default_value = {
        .attr_max_len = CHAR_VALUE_MAX_LEN,
        .attr_len = 0,
        .attr_value = NULL,
    };
    esp_err_t add_char_ret;

    // Charakterystyka SSID
    default_value.attr_value = ssid_value.value;
    default_value.attr_len = ssid_value.length;
    add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PAIRING_PROFILE_APP_ID].service_handle, 
                           (esp_bt_uuid_t[]){{.len = ESP_UUID_LEN_16, .uuid.uuid16 = GATTS_CHAR_UUID_SSID}}, 
                           ESP_GATT_PERM_WRITE, ESP_GATT_CHAR_PROP_BIT_WRITE, 
                           &default_value, NULL);
    if(add_char_ret){
        ESP_LOGE(TAG, "add char failed, error code =%x",add_char_ret);
    }

    // Charakterystyka Password
    default_value.attr_value = password_value.value;
    default_value.attr_len = password_value.length;
    add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PAIRING_PROFILE_APP_ID].service_handle, 
                           (esp_bt_uuid_t[]){{.len = ESP_UUID_LEN_16, .uuid.uuid16 = GATTS_CHAR_UUID_PASSWORD}}, 
                           ESP_GATT_PERM_WRITE, ESP_GATT_CHAR_PROP_BIT_WRITE, 
                           &default_value, NULL);
    if(add_char_ret != ESP_OK){
        ESP_LOGE(TAG, "add char failed, error code =%x",add_char_ret);
    }

    // Charakterystyka User
    default_value.attr_value = user_id_value.value;
    default_value.attr_len = user_id_value.length;
    add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PAIRING_PROFILE_APP_ID].service_handle, 
                           (esp_bt_uuid_t[]){{.len = ESP_UUID_LEN_16, .uuid.uuid16 = GATTS_CHAR_UUID_USER_ID}}, 
                           ESP_GATT_PERM_WRITE, ESP_GATT_CHAR_PROP_BIT_WRITE, 
                           &default_value, NULL);
    if(add_char_ret != ESP_OK){
        ESP_LOGE(TAG, "add char failed, error code =%x",add_char_ret);
    }

    // // Charakterystyka MQTT username
    // default_value.attr_value = mqtt_username_value.value;
    // default_value.attr_len = mqtt_username_value.length;
    // add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PAIRING_PROFILE_APP_ID].service_handle, 
    //                        (esp_bt_uuid_t[]){{.len = ESP_UUID_LEN_16, .uuid.uuid16 = GATTS_CHAR_UUID_MQTT_USERNAME}}, 
    //                        ESP_GATT_PERM_WRITE, ESP_GATT_CHAR_PROP_BIT_WRITE, 
    //                        &default_value, NULL);
    // if(add_char_ret != ESP_OK){
    //     ESP_LOGE(TAG, "add char failed, error code =%x",add_char_ret);
    // }
    
    // // Charakterystyka MQTT
    // default_value.attr_value = mqtt_url_value.value;
    // default_value.attr_len = mqtt_url_value.length;
    // add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PAIRING_PROFILE_APP_ID].service_handle, 
    //                        (esp_bt_uuid_t[]){{.len = ESP_UUID_LEN_16, .uuid.uuid16 = GATTS_CHAR_UUID_MQTT_URL}}, 
    //                        ESP_GATT_PERM_WRITE, ESP_GATT_CHAR_PROP_BIT_WRITE, 
    //                        &default_value, NULL);
    // if(add_char_ret != ESP_OK){
    //     ESP_LOGE(TAG, "add char failed, error code =%x",add_char_ret);
    // }
    
    // // Charakterystyka MQTT password
    // default_value.attr_value = mqtt_password_value.value;
    // default_value.attr_len = mqtt_password_value.length;
    // add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PAIRING_PROFILE_APP_ID].service_handle, 
    //                        (esp_bt_uuid_t[]){{.len = ESP_UUID_LEN_16, .uuid.uuid16 = GATTS_CHAR_UUID_MQTT_PASSWORD}}, 
    //                        ESP_GATT_PERM_WRITE, ESP_GATT_CHAR_PROP_BIT_WRITE, 
    //                        &default_value, NULL);   
    // if(add_char_ret != ESP_OK){
    //     ESP_LOGE(TAG, "add char failed, error code =%x",add_char_ret);
    // }


    // Charakterystyka Confirm
    default_value.attr_value = confirm_value.value;
    default_value.attr_len = confirm_value.length;
    add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PAIRING_PROFILE_APP_ID].service_handle, 
                           (esp_bt_uuid_t[]){{.len = ESP_UUID_LEN_16, .uuid.uuid16 = GATTS_CHAR_UUID_CONFIRM}}, 
                           ESP_GATT_PERM_WRITE, ESP_GATT_CHAR_PROP_BIT_WRITE, 
                           &default_value, NULL);
    if(add_char_ret != ESP_OK){
        ESP_LOGE(TAG, "add char failed, error code =%x",add_char_ret);
    }

    //Charakterystyka NOTIFY
    default_value.attr_value = confirm_value.value;
    default_value.attr_len = confirm_value.length;
    add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PAIRING_PROFILE_APP_ID].service_handle,
                            (esp_bt_uuid_t[]){{.len = ESP_UUID_LEN_16, .uuid.uuid16 = GATTS_CHAR_UUID_NOTIFY}},
                            ESP_GATT_PERM_WRITE | ESP_GATT_PERM_READ,
                            ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                            &default_value, NULL);
    if(add_char_ret != ESP_OK){
        ESP_LOGE(TAG, "add char failed, error code =%x",add_char_ret);
    }
}

void save_config_to_nvs(){
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open("config", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }

    if (ssid_value.length > 0) {
        err = nvs_set_str(nvs_handle, "ssid", (const char *)ssid_value.value);
        if (err != ESP_OK) {
            ESP_LOGE("NVS", "Failed to save SSID: %s", esp_err_to_name(err));
        }
    }

    if (password_value.length > 0) {
        err = nvs_set_str(nvs_handle, "password", (const char *)password_value.value);
        if (err != ESP_OK) {
            ESP_LOGE("NVS", "Failed to save Password: %s", esp_err_to_name(err));
        }
    }

    if (user_id_value.length > 0) {
        err = nvs_set_str(nvs_handle, "user_id", (const char *)user_id_value.value);
        if (err != ESP_OK) {
            ESP_LOGE("NVS", "Failed to save User: %s", esp_err_to_name(err));
        }
    }

    // if (mqtt_username_value.length > 0) {
    //     err = nvs_set_str(nvs_handle, "mqtt_username", (const char *)mqtt_username_value.value);
    //     if (err != ESP_OK) {
    //         ESP_LOGE("NVS", "Failed to save MQTT username: %s", esp_err_to_name(err));
    //     }
    // }

    // if (mqtt_password_value.length > 0) {
    //     err = nvs_set_str(nvs_handle, "mqtt_password", (const char *)mqtt_password_value.value);
    //     if (err != ESP_OK) {
    //         ESP_LOGE("NVS", "Failed to save MQTT password: %s", esp_err_to_name(err));
    //     }
    // }

    // if (mqtt_url_value.length > 0) {
    //     err = nvs_set_str(nvs_handle, "mqtt_url", (const char *)mqtt_url_value.value);
    //     if (err != ESP_OK) {
    //         ESP_LOGE("NVS", "Failed to save MQTT URL: %s", esp_err_to_name(err));
    //     }
    // }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to commit changes: %s", esp_err_to_name(err));
    }

    nvs_close(nvs_handle);
    ESP_LOGI("NVS", "Configuration values saved successfully.");
}

void notify_ble_wifi_begin(){
    uint8_t notify_data[2] = {0,0};
    ESP_LOGI(TAG, "Sending notify data ble WiFi config begin: ");
    esp_log_buffer_hex(TAG, notify_data, sizeof(notify_data));
    esp_ble_gatts_send_indicate(
        notify_gatts_if, 
        notify_conn_id,
        notify_handle, 
        sizeof(notify_data), 
        notify_data, 
        true
    );
}

void notify_ble_wifi_connected(){
    uint8_t notify_data[2] = {0,1};
    ESP_LOGI(TAG, "Sending notify data ble WiFi config begin: ");
    esp_log_buffer_hex(TAG, notify_data, sizeof(notify_data));
    esp_ble_gatts_send_indicate(
        notify_gatts_if, 
        notify_conn_id,
        notify_handle, 
        sizeof(notify_data), 
        notify_data, 
        true
    );
}

void notify_ble_mqtt_failure(){
    uint8_t notify_data[2] = {1,0};
    ESP_LOGI(TAG, "Sending notify data ble - MQTT config failure: ");
    esp_log_buffer_hex(TAG, notify_data, sizeof(notify_data));
    esp_ble_gatts_send_indicate(
        notify_gatts_if, 
        notify_conn_id,
        notify_handle, 
        sizeof(notify_data), 
        notify_data, 
        true
    );
}

void notify_ble_mqtt_success(){
    uint8_t notify_data[2] = {1,1};
    ESP_LOGI(TAG, "Sending notify data ble - MQTT config success: ");
    esp_log_buffer_hex(TAG, notify_data, sizeof(notify_data));
    esp_ble_gatts_send_indicate(
        notify_gatts_if, 
        notify_conn_id,
        notify_handle, 
        sizeof(notify_data), 
        notify_data, 
        true
    );
}

void notify_ble_config_failure(){
    ble_configuration_done = false;
    uint8_t notify_data[2] = {2,0};
    ESP_LOGI(TAG, "Sending notify data ble - WiFi config failure: ");
    esp_log_buffer_hex(TAG, notify_data, sizeof(notify_data));
    esp_ble_gatts_send_indicate(
        notify_gatts_if, 
        notify_conn_id,
        notify_handle, 
        sizeof(notify_data), 
        notify_data, 
        true
    );
}

void notify_ble_config_success(){
    uint8_t notify_data[2] = {2,1};
    ESP_LOGI(TAG, "Sending notify data ble - WiFi config success: ");
    esp_log_buffer_hex(TAG, notify_data, sizeof(notify_data));
    esp_ble_gatts_send_indicate(
        notify_gatts_if, 
        notify_conn_id, 
        notify_handle, 
        sizeof(notify_data), 
        notify_data, 
        true
    );
}


bool check_config_values(){
    if(ssid_value.length == 0){
        ESP_LOGE(TAG, "Tried to save empty SSID");
        return false;
    }
    if(password_value.length == 0){
        ESP_LOGW(TAG, "Empty Wifi password - proceed");
    }
    if(user_id_value.length == 0){
        ESP_LOGE(TAG, "Tried to save empty user name");
        return false;
    }
    // if(mqtt_username_value.length == 0){
    //     ESP_LOGE(TAG, "Tried to save empty MQTT username");
    //     return false;
    // }
    // if(mqtt_password_value.length == 0){
    //     ESP_LOGE(TAG, "Tried to save empty MQTT password");
    //     return false;
    // }
    // if(mqtt_url_value.length == 0){
    //     ESP_LOGE(TAG, "Tried to save empty MQTT URL");
    //     return false;
    // }
    ESP_LOGI(TAG, "Configuration correct");
    // notify_ble_wifi_begin();
    return true;
}

// Handling GAP events - connection related
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
#ifdef CONFIG_SET_RAW_ADV_DATA
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0) {
            if (esp_ble_gap_start_advertising(&adv_params) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to start advertising (RAW ADV DATA)");
            }
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0) {
            if (esp_ble_gap_start_advertising(&adv_params) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to start advertising (RAW SCAN RSP DATA)");
            }
        }
        break;
#else
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0) {
            if (esp_ble_gap_start_advertising(&adv_params) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to start advertising");
            } else {
                ESP_LOGI(TAG, "Advertising started");
            }
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0) {
            if (esp_ble_gap_start_advertising(&adv_params) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to start advertising (SCAN RSP DATA)");
            } else {
                ESP_LOGI(TAG, "Advertising restarted after scan response set");
            }
        }
        break;
#endif
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(TAG, "Advertising start failed");
        } else {
            ESP_LOGI(TAG, "Advertising started successfully");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(TAG, "Advertising stop failed");
        } else {
            ESP_LOGI(TAG, "Advertising stopped successfully");
        }
        break;
    // case ESP_GATTS_DISCONNECT_EVT:
    //     ESP_LOGI(TAG, "Device disconnected, restarting advertising");
    //     if (esp_ble_gap_start_advertising(&adv_params) != ESP_OK) {
    //         ESP_LOGE(TAG, "Failed to restart advertising after disconnection");
    //     }
    //     break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(TAG, "Updated connection params: min_int = %d, max_int = %d, conn_int = %d, latency = %d, timeout = %d",
                 param->update_conn_params.min_int,
                 param->update_conn_params.max_int,
                 param->update_conn_params.conn_int,
                 param->update_conn_params.latency,
                 param->update_conn_params.timeout);
        break;
    default:
        ESP_LOGW(TAG, "Unhandled GAP event: %d", event);
        break;
    }
}


// Handle write long event
static void handle_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    // ESP_LOGI(TAG, "Start write handle");
    esp_gatt_status_t status = ESP_GATT_OK;
    if (param->write.need_rsp){
        if (param->write.is_prep) {
            ESP_LOGI(TAG, "Write - is prepare write");
            if (param->write.offset > PREPARE_BUF_MAX_SIZE) {
                status = ESP_GATT_INVALID_OFFSET;
            } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
                status = ESP_GATT_INVALID_ATTR_LEN;
            }
            // allocate space for response
            if (status == ESP_GATT_OK && prepare_write_env->prepare_buf == NULL) {
                prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE*sizeof(uint8_t));
                prepare_write_env->prepare_len = 0;
                if (prepare_write_env->prepare_buf == NULL) {
                    ESP_LOGE(TAG, "Gatt_server prep no mem");
                    status = ESP_GATT_NO_RESOURCES;
                }
            }
            // try to allocate the response
            esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
            if (gatt_rsp) {
                gatt_rsp->attr_value.len = param->write.len;
                gatt_rsp->attr_value.handle = param->write.handle;
                gatt_rsp->attr_value.offset = param->write.offset;
                gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
                // copy data
                memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
                // send response
                esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
                if (response_err != ESP_OK){
                    ESP_LOGE(TAG, "Send response error\n");
                }
                free(gatt_rsp);
            } else {
                ESP_LOGE(TAG, "malloc failed, no resource to send response error\n");
                status = ESP_GATT_NO_RESOURCES;
            }
            if (status != ESP_GATT_OK){
                return;
            }
            memcpy(prepare_write_env->prepare_buf + param->write.offset,
                   param->write.value,
                   param->write.len);
            prepare_write_env->prepare_len += param->write.len;
            prepare_write_env->handle = param->write.handle;

            // ESP_LOGI(TAG, "Current write buffer: %s", prepare_write_env->prepare_buf);
        }
        // IS NOT PREP WRITE, 
        else{
            ESP_LOGI(TAG, "Write - is NOT prepare write, saving value.");
            if(status == ESP_GATT_OK){
                // Obsługa zapisu dla odpowiedniej charakterystyki
                if (param->write.handle == wifi_ssid_handle) {
                    strncpy((char *)ssid_value.value, (char *)param->write.value, param->write.len);
                    ssid_value.length = param->write.len;
                    ssid_value.value[param->write.len] = '\0';
                    ESP_LOGI(TAG, "SSID updated: %s", ssid_value.value);
                }
                else if (param->write.handle == wifi_password_handle) {
                    strncpy((char *)password_value.value, (char *)param->write.value, param->write.len);
                    password_value.length = param->write.len;
                    password_value.value[param->write.len] = '\0';
                    ESP_LOGI(TAG, "Password updated: %s", password_value.value);
                } 
                else if (param->write.handle == user_id_handle) {
                    strncpy((char *)user_id_value.value, (char *)param->write.value, param->write.len);
                    user_id_value.length = param->write.len;
                    user_id_value.value[param->write.len] = '\0';
                    ESP_LOGI(TAG, "User ID updated: %s", user_id_value.value);
                }
                // else if (param->write.handle == mqtt_username_handle) {
                //     strncpy((char *)mqtt_username_value.value, (char *)param->write.value, param->write.len);
                //     mqtt_username_value.length = param->write.len;
                //     mqtt_username_value.value[param->write.len] = '\0';
                //     ESP_LOGI(TAG, "MQTT username updated: %s", mqtt_username_value.value);
                // }
                // else if (param->write.handle == mqtt_url_handle) {
                //     strncpy((char *)mqtt_url_value.value, (char *)param->write.value, param->write.len);
                //     mqtt_url_value.length = param->write.len;
                //     mqtt_url_value.value[param->write.len] = '\0';
                //     ESP_LOGI(TAG, "MQTT URL updated: %s", mqtt_url_value.value);
                // } 
                // else if (param->write.handle == mqtt_password_handle) {
                //     strncpy((char *)mqtt_password_value.value, (char *)param->write.value, param->write.len);
                //     mqtt_password_value.length = param->write.len;
                //     mqtt_password_value.value[param->write.len] = '\0';
                //     ESP_LOGI(TAG, "MQTT password updated: %s", mqtt_password_value.value);
                // }
                else{
                    ESP_LOGE(TAG, "Unknown characteristic.");
                }
            }
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, NULL);
        }
    }
}

void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    // check if write is confirmed
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC){
        ESP_LOGI(TAG,"Prep write done, saving value.");
        // Obsługa zapisu dla odpowiedniej charakterystyki
        if (prepare_write_env->handle == wifi_ssid_handle) {
            if(prepare_write_env->prepare_len <= CHAR_VALUE_MAX_LEN){
                strncpy((char *)ssid_value.value, (char *)prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
                ssid_value.length = prepare_write_env->prepare_len;
                ssid_value.value[prepare_write_env->prepare_len] = '\0'; // Dodanie null-terminatora
                ESP_LOGI(TAG, "SSID updated: %s", ssid_value.value);
            }
            else {
                ESP_LOGE(TAG, "New value is too long, max lenght: %d, actual: %d", CHAR_VALUE_MAX_LEN ,prepare_write_env->prepare_len);
            }
        } 
        else if (prepare_write_env->handle == wifi_password_handle) {
            if(prepare_write_env->prepare_len <= CHAR_VALUE_MAX_LEN){
                strncpy((char *)password_value.value, (char *)prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
                password_value.length = prepare_write_env->prepare_len;
                password_value.value[prepare_write_env->prepare_len] = '\0';
                ESP_LOGI(TAG, "Password updated: %s", password_value.value);
            }
            else {
                ESP_LOGE(TAG, "New value is too long, max lenght: %d, actual: %d", CHAR_VALUE_MAX_LEN ,prepare_write_env->prepare_len);
            }
        } 
        else if (prepare_write_env->handle == user_id_handle) {
            if(prepare_write_env->prepare_len <= CHAR_VALUE_MAX_LEN){
                strncpy((char *)user_id_value.value, (char *)prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
                user_id_value.length = prepare_write_env->prepare_len;
                user_id_value.value[prepare_write_env->prepare_len] = '\0';
                ESP_LOGI(TAG, "User ID updated: %s", user_id_value.value);
            }
            else {
                ESP_LOGE(TAG, "New value is too long, max lenght: %d, actual: %d", CHAR_VALUE_MAX_LEN ,prepare_write_env->prepare_len);
            }
        }
        // else if (prepare_write_env->handle == mqtt_username_handle) {
        //     if(prepare_write_env->prepare_len <= CHAR_VALUE_MAX_LEN){
        //         strncpy((char *)mqtt_username_value.value, (char *)prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
        //         mqtt_username_value.length = prepare_write_env->prepare_len;
        //         mqtt_username_value.value[prepare_write_env->prepare_len] = '\0';
        //         ESP_LOGI(TAG, "MQTT username updated: %s", mqtt_username_value.value);
        //     }
        //     else {
        //         ESP_LOGE(TAG, "New value is too long, max lenght: %d, actual: %d", CHAR_VALUE_MAX_LEN ,prepare_write_env->prepare_len);
        //     }
        // }
        // else if (prepare_write_env->handle == mqtt_url_handle) {
        //     if(prepare_write_env->prepare_len <= CHAR_VALUE_MAX_LEN){
        //     strncpy((char *)mqtt_url_value.value, (char *)prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
        //     mqtt_url_value.length = prepare_write_env->prepare_len;
        //     mqtt_url_value.value[prepare_write_env->prepare_len] = '\0';
        //     ESP_LOGI(TAG, "MQTT URL updated: %s", mqtt_url_value.value);
        //     }
        //     else {
        //         ESP_LOGE(TAG, "New value is too long, max lenght: %d, actual: %d", CHAR_VALUE_MAX_LEN ,prepare_write_env->prepare_len);
        //     }
        // } 
        // else if (prepare_write_env->handle == mqtt_password_handle) {
        //     if(prepare_write_env->prepare_len <= CHAR_VALUE_MAX_LEN){
        //         strncpy((char *)mqtt_password_value.value, (char *)prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
        //         mqtt_password_value.length = prepare_write_env->prepare_len;
        //         mqtt_password_value.value[prepare_write_env->prepare_len] = '\0';
        //         ESP_LOGI(TAG, "MQTT username updated: %s", mqtt_password_value.value);
        //     }
        //     else {
        //         ESP_LOGE(TAG, "New value is too long, max lenght: %d, actual: %d", CHAR_VALUE_MAX_LEN ,prepare_write_env->prepare_len);
        //     }
        // }
        else{
            ESP_LOGE(TAG, "Unknown characteristic.");
        }
    }else{
        ESP_LOGI(TAG,"ESP_GATT_PREP_WRITE_CANCEL");
    }
    // free memory for long write response
    // prepare buffer for next write operation
    if (prepare_write_env->prepare_buf) {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
    prepare_write_env->handle = 0;

}


static void gatts_pairing_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
    // Register new application - on app start when new application is registered
    case ESP_GATTS_REG_EVT:
        // Set all application params
        ESP_LOGI(TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);
        gl_profile_tab[PAIRING_PROFILE_APP_ID].service_id.is_primary = true;
        gl_profile_tab[PAIRING_PROFILE_APP_ID].service_id.id.inst_id = 0x00;
        gl_profile_tab[PAIRING_PROFILE_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PAIRING_PROFILE_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID;
        
        gl_profile_tab[PAIRING_PROFILE_APP_ID].gatts_if = gatts_if;

        esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(TEST_DEVICE_NAME);
        if (set_dev_name_ret){
            ESP_LOGE(TAG, "set device name failed, error code = %x", set_dev_name_ret);
        }
        // For advertising using raw data
#ifdef CONFIG_SET_RAW_ADV_DATA
        esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
        if (raw_adv_ret){
            ESP_LOGE(TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
        }
        adv_config_done |= adv_config_flag;
        esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
        if (raw_scan_ret){
            ESP_LOGE(TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
        }
        adv_config_done |= scan_rsp_config_flag;
#else
        //config adv data
        esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
        if (ret){
            ESP_LOGE(TAG, "config adv data failed, error code = %x", ret);
        }
        adv_config_done |= adv_config_flag;
        //config scan response data
        ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
        if (ret){
            ESP_LOGE(TAG, "config scan response data failed, error code = %x", ret);
        }
        adv_config_done |= scan_rsp_config_flag;

#endif
        esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PAIRING_PROFILE_APP_ID].service_id, GATTS_NUM_HANDLE);
        break;

    // Handling read event - manual, return mock response
    case ESP_GATTS_READ_EVT: {
        ESP_LOGI(TAG, "GATT_READ_EVT, conn_id %d, trans_id %" PRIu32 ", handle %d", param->read.conn_id, param->read.trans_id, param->read.handle);
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        //Create mock response
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len = 4;
        rsp.attr_value.value[0] = 0x00;
        rsp.attr_value.value[1] = 0x00;
        rsp.attr_value.value[2] = 0x00;
        rsp.attr_value.value[3] = 0x00;
        // Send response
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);
        break;
    }
    case ESP_GATTS_WRITE_EVT: 
        ESP_LOGI(TAG, "ESP_GATTS_WRITE_EVT, handle: %d, value: %s",
                    param->write.handle, param->write.value);

        if (param->write.handle == gl_profile_tab[PAIRING_PROFILE_APP_ID].descr_handle) {
            uint16_t notify_enable = param->write.value[1] << 8 | param->write.value[0];

            if (notify_enable == 0x0001) {
                uint8_t notify_data[2] = {0,0};
                
                notify_gatts_if = gatts_if;
                notify_conn_id = param->read.conn_id;      
                esp_ble_gatts_send_indicate(
                    gatts_if,
                    param->write.conn_id,
                    notify_handle,
                    sizeof(notify_data),
                    notify_data,
                    false);
                ESP_LOGI(TAG, "Notifications enabled for handle %d", param->write.handle);

            } else if (notify_enable == 0x0000) {
                ESP_LOGI(TAG, "Notifications disabled for handle %d", param->write.handle);
            } else {
                ESP_LOGW(TAG, "Unknown value for CCCD: 0x%04x", notify_enable);
            }
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        } 
        // // Obsługa zapisu dla odpowiedniej charakterystyki
        // if (param->write.handle == wifi_ssid_handle) {
        //     strncpy((char *)ssid_value.value, (char *)param->write.value, param->write.len);
        //     ssid_value.length = param->write.len;
        //     ssid_value.value[param->write.len] = '\0'; // Dodanie null-terminatora
        //     ESP_LOGI(TAG, "SSID updated: %s", ssid_value.value);
        //     esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, &ssid_value.value);
        // } 
        // else if (param->write.handle == wifi_password_handle) {
        //     strncpy((char *)password_value.value, (char *)param->write.value, param->write.len);
        //     password_value.length = param->write.len;
        //     password_value.value[param->write.len] = '\0';
        //     ESP_LOGI(TAG, "Password updated: %s", password_value.value);
        //     esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, &password_value.value);
        // } 
        // // else if (param->write.handle == username_handle) {
        // //     strncpy((char *)user_value.value, (char *)param->write.value, param->write.len);
        // //     user_value.length = param->write.len;
        // //     user_value.value[param->write.len] = '\0';
        // //     ESP_LOGI(TAG, "Username updated: %s", user_value.value);
        // //     esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, &user_value.value);
        // // } 
        // else if (param->write.handle == mqtt_username_handle) {
        //     strncpy((char *)mqtt_username_value.value, (char *)param->write.value, param->write.len);
        //     mqtt_username_value.length = param->write.len;
        //     mqtt_username_value.value[param->write.len] = '\0';
        //     ESP_LOGI(TAG, "MQTT username updated: %s", mqtt_username_value.value);
        //     esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, &mqtt_username_value.value);
        // }
        // else if (param->write.handle == mqtt_url_handle) {
        //     strncpy((char *)mqtt_url_value.value, (char *)param->write.value, param->write.len);
        //     mqtt_url_value.length = param->write.len;
        //     mqtt_url_value.value[param->write.len] = '\0';
        //     ESP_LOGI(TAG, "MQTT URL updated: %s", mqtt_url_value.value);
        //     esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        // } 
        // else if (param->write.handle == mqtt_password_handle) {
        //     strncpy((char *)mqtt_password_value.value, (char *)param->write.value, param->write.len);
        //     mqtt_password_value.length = param->write.len;
        //     mqtt_password_value.value[param->write.len] = '\0';
        //     ESP_LOGI(TAG, "MQTT username updated: %s", mqtt_password_value.value);
        //     esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, &mqtt_password_value.value);
        // }

        // // Obsługa zapisu dla odpowiedniej charakterystyki
        else if (param->write.handle != confirm_handle) {
            ESP_LOGI(TAG, "Handle write event...");
            handle_write_event_env(gatts_if, &pairing_prepare_write_env, param);
            // strncpy((char *)ssid_value.value, (char *)param->write.value, param->write.len);
            // ssid_value.length = param->write.len;
            // ssid_value.value[param->write.len] = '\0'; // Dodanie null-terminatora
            // ESP_LOGI(TAG, "SSID updated: %s", ssid_value.value);
            // esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, &ssid_value.value);
        } 
        else if (param->write.handle == confirm_handle) {
            if (param->write.len > 0 && param->write.value[0] == 1) {
                ESP_LOGI(TAG, "Configuration confirmed. Starting setup...");

                if(check_config_values()){
                   save_config_to_nvs(); 
                   ble_configuration_done = true;
                }
                else {
                    ESP_LOGW(TAG, "Not all config characteristics are set correctly.");
                }
                esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
                // // Przykład dalszej logiki
                // // wifi_init_sta(ble_data.ssid, ble_data.password);
                // // mqtt_connect(ble_data.mqtt, ble_data.user, ble_data.password);

                // // Wyłącz serwer BLE (opcjonalnie)
                // ble_server_stop();
            }
            else {
                ESP_LOGW(TAG, "Invalid confirm value");
                esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
            }
        }
        else if (param->write.handle == notify_handle) {
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        }
        else {
            ESP_LOGI(TAG, "Unknown characteristic: %d",param->write.handle);
        }

        // ESP_LOGI(TAG, "Need response:  %d", param->write.need_rsp);
        break;
    case ESP_GATTS_EXEC_WRITE_EVT:
        ESP_LOGI(TAG,"ESP_GATTS_EXEC_WRITE_EVT");
        esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        example_exec_write_event_env(&pairing_prepare_write_env, param);
        break;
    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
        break;
    case ESP_GATTS_UNREG_EVT:
        break;
    // After successful service creation
    case ESP_GATTS_CREATE_EVT:
        // Configure characteristic
        ESP_LOGI(TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d", param->create.status, param->create.service_handle);

        gl_profile_tab[PAIRING_PROFILE_APP_ID].service_handle = param->create.service_handle;
        gl_profile_tab[PAIRING_PROFILE_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PAIRING_PROFILE_APP_ID].char_uuid.uuid.uuid16 = GATTS_SERVICE_UUID;

        // Start service
        esp_ble_gatts_start_service(gl_profile_tab[PAIRING_PROFILE_APP_ID].service_handle);

        configure_characteristics();

        break;
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
        break;
    // When new characteristic is added
    case ESP_GATTS_ADD_CHAR_EVT: {
        ESP_LOGI(TAG, "ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d", param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        
        // Sprawdź UUID, aby przypisać odpowiedni handle
        if (param->add_char.char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_SSID) {
            wifi_ssid_handle = param->add_char.attr_handle;
            ESP_LOGI(TAG, "SSID handle: %d", wifi_ssid_handle);
        } else if (param->add_char.char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_PASSWORD) {
            wifi_password_handle = param->add_char.attr_handle;
            ESP_LOGI(TAG, "Password handle: %d", wifi_password_handle);
        } else if (param->add_char.char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_USER_ID) {
            user_id_handle = param->add_char.attr_handle;
            ESP_LOGI(TAG, "User ID handle: %d", user_id_handle);
        // } else if (param->add_char.char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_MQTT_USERNAME) {
        //     mqtt_username_handle = param->add_char.attr_handle;
        //     ESP_LOGI(TAG, "MQTT username handle: %d", mqtt_username_handle);
        // } else if (param->add_char.char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_MQTT_URL) {
        //     mqtt_url_handle = param->add_char.attr_handle;
        //     ESP_LOGI(TAG, "MQTT URL handle: %d", mqtt_url_handle);
        // } else if (param->add_char.char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_MQTT_PASSWORD) {
        //     mqtt_password_handle = param->add_char.attr_handle;
        //     ESP_LOGI(TAG, "MQTT password: %d", mqtt_password_handle);
        } else if (param->add_char.char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_CONFIRM) {
            confirm_handle = param->add_char.attr_handle;
            ESP_LOGI(TAG, "Confirm handle: %d", confirm_handle);
        } else if (param->add_char.char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_NOTIFY) {

            esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(
                gl_profile_tab[PAIRING_PROFILE_APP_ID].service_handle,
                (esp_bt_uuid_t[]){{.len = ESP_UUID_LEN_16, .uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG}},
                ESP_GATT_PERM_WRITE | ESP_GATT_PERM_READ,
                &(esp_attr_value_t){.attr_max_len = 30, .attr_len = 6, .attr_value = (uint8_t *)"Notify"},
                NULL
            );

            if (add_descr_ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to add descriptor: %d", add_descr_ret);
            }
                
            notify_handle = param->add_char.attr_handle;
            ESP_LOGI(TAG, "Notify handle: %d", notify_handle);
        }
        break;
    }
    // When adding new descriptor
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
        // Save descriptor handle
        gl_profile_tab[PAIRING_PROFILE_APP_ID].descr_handle = param->add_char_descr.attr_handle;
        ESP_LOGI(TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d",
                 param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
        break;
    case ESP_GATTS_DELETE_EVT:
        break;
    case ESP_GATTS_START_EVT:
        ESP_LOGI(TAG, "SERVICE_START_EVT, status %d, service_handle %d",
                 param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_STOP_EVT:
        break;
    // On connection
    case ESP_GATTS_CONNECT_EVT: {
        // connection parameters
        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
        conn_params.latency = 0;
        conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
        conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
        ESP_LOGI(TAG, "ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
                 param->connect.conn_id,
                 param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
                 param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
        gl_profile_tab[PAIRING_PROFILE_APP_ID].conn_id = param->connect.conn_id;

        //start sent the update connection parameters to the peer device.
        // Triggers : ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT
        esp_ble_gap_update_conn_params(&conn_params);
        break;
    }
    // On disconnect
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x", param->disconnect.reason);
        esp_ble_gap_start_advertising(&adv_params);
        break;
    // On configuration
    case ESP_GATTS_CONF_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_CONF_EVT, status %d attr_handle %d", param->conf.status, param->conf.handle);
        if (param->conf.status != ESP_GATT_OK){
            esp_log_buffer_hex(TAG, param->conf.value, param->conf.len);
        }
        break;
    case ESP_GATTS_OPEN_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_OPEN_EVT");
        break;
    case ESP_GATTS_CANCEL_OPEN_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_CANCEL_OPEN_EVT");
        break;
    case ESP_GATTS_CLOSE_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_CLOSE_EVT");
        break;
    case ESP_GATTS_LISTEN_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_LISTEN_EVT");
        break;
    case ESP_GATTS_CONGEST_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_CONGEST_EVT");
        break;
    default:
        break;
    }
}

bool check_ble_config_done(void){
    return ble_configuration_done;
}

void ble_server_init() {
    ESP_LOGI(TAG, "Initializing BLE.");
    ble_configuration_done = false;
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    esp_bluedroid_init();
    esp_bluedroid_enable();

    esp_ble_gatts_register_callback(gatts_pairing_profile_event_handler);
    esp_ble_gap_register_callback(gap_event_handler);
    esp_ble_gatts_app_register(PAIRING_PROFILE_APP_ID);

    ble_initialized = true;
    ble_enabled = true;
}

void ble_server_stop() {
    ESP_LOGI(TAG, "Stopping BLE.");

    if (!ble_initialized) {
        ESP_LOGW(TAG, "BLE not initialized, skipping stop.");
        return;
    }
    if (!ble_enabled) {
        ESP_LOGW(TAG, "BLE already stopped, skipping stop.");
        return;
    }

    esp_err_t error = ESP_OK;
    error = esp_ble_gap_stop_advertising();
    if(error != ESP_OK) { ESP_LOGE(TAG, "Error when trying to esp_ble_gap_stop_advertising: %d", error); return;} 
    ESP_LOGI(TAG, "esp_ble_gap_stop_advertising OK");

    error = esp_ble_gatts_app_unregister(gl_profile_tab[PAIRING_PROFILE_APP_ID].gatts_if);
    if(error != ESP_OK) { ESP_LOGE(TAG, "Error when trying to esp_ble_gatts_app_unregister: %d", error); return;} 
    ESP_LOGI(TAG, "esp_ble_gatts_app_unregister OK");

    error = esp_bluedroid_disable();
    if(error != ESP_OK) { ESP_LOGE(TAG, "Error when trying to esp_bluedroid_disable: %d", error); return;} 
    ESP_LOGI(TAG, "esp_bluedroid_disable OK");

    error = esp_bluedroid_deinit();
    if(error != ESP_OK) { ESP_LOGE(TAG, "Error when trying to esp_bluedroid_deinit: %d", error); return;}
    ESP_LOGI(TAG, "esp_bluedroid_deinit OK");

    error = esp_bt_controller_disable();
    if(error != ESP_OK) { ESP_LOGE(TAG, "Error when trying to esp_bt_controller_disable: %d", error); return;} 
    ESP_LOGI(TAG, "esp_bt_controller_disable OK");

    error = esp_bt_controller_deinit();
    if(error != ESP_OK) { ESP_LOGE(TAG, "Error when trying to esp_bt_controller_deinit: %d", error); return;} 
    ESP_LOGI(TAG, "esp_bt_controller_deinit OK");


    ble_enabled = false;
    ble_initialized = false;
}


