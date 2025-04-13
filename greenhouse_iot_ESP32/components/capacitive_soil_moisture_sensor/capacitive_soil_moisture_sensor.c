#include "capacitive_soil_moisture_sensor.h"
#include "esp_log.h"

static const char *TAG = "CAPACITIVE_SOIL_MOISTURE_SENSOR";

static uint32_t oversample_read(capacitive_soil_moisture_sensor_t *sensor, uint16_t samples) {
    uint32_t total = 0;
    for (uint16_t i = 0; i < samples; i++) {
        total += adc1_get_raw(sensor->adc_channel);
    }
    total /= samples;
    uint32_t voltage = esp_adc_cal_raw_to_voltage(total, &(sensor->adc_chars));
    return voltage;
}

int capacitive_soil_moisture_sensor_init(capacitive_soil_moisture_sensor_t *sensor, adc_unit_t adc_unit, adc1_channel_t adc_channel, adc_atten_t adc_atten_db, adc_bits_width_t adc_bits_width, int default_vref) {
    if (!sensor) {
        ESP_LOGE(TAG, "Sensor struct pointer is NULL");
        return -1;
    }

    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_characterize(adc_unit, adc_atten_db, adc_bits_width, default_vref, &adc_chars);

    sensor->adc_channel = adc_channel;
    sensor->adc_chars = adc_chars;
    sensor->oversampling = CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_4X;
    sensor->last_raw_value = 0;

    esp_err_t ret = adc1_config_channel_atten(sensor->adc_channel, adc_atten_db);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Configuration error for ADC: %s", esp_err_to_name(ret));
        return -2;
    }

    ESP_LOGI(TAG, "Sensor initiated successfully on ADC channel: %d", sensor->adc_channel);
    return 0;
}

uint32_t capacitive_soil_moisture_sensor_read_value(capacitive_soil_moisture_sensor_t *sensor) {
    if (!sensor) {
        ESP_LOGE(TAG, "");
        return 0;
    }

    uint32_t value;
    switch (sensor->oversampling) {
        case CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_NONE:
            value = sensor->last_raw_value;
            break;
        case CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_2X:
        case CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_4X:
        case CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_8X:
        case CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_16X:
        case CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_32X:
        case CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_64X:
            value = oversample_read(sensor, sensor->oversampling);
            break;
        default:
            ESP_LOGW(TAG, "Incorrect oversampling level, using default");
            value = adc1_get_raw(sensor->adc_channel);
            break;
    }

    sensor->last_raw_value = value;
    return value;
}

void capacitive_soil_moisture_sensor_set_oversampling(capacitive_soil_moisture_sensor_t *sensor, capacitive_soil_moisture_sensor_oversampling_t oversampling) {
    if (!sensor) {
        ESP_LOGE(TAG, "Sensor struct pointer is Null");
        return;
    }

    if (oversampling == CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_NONE || 
        oversampling == CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_2X   || 
        oversampling == CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_4X   || 
        oversampling == CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_8X   ||
        oversampling == CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_16X   || 
        oversampling == CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_32X   || 
        oversampling == CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_64X) {
        sensor->oversampling = oversampling;
        ESP_LOGI(TAG, "Oversampling set to: %dx", oversampling);
    } else {
        ESP_LOGW(TAG, "Incorrect oversampling level: %d", oversampling);
    }
}

uint32_t capacitive_soil_moisture_sensor_compensate(uint32_t raw_value) {
    int32_t compensated = 100 - (int32_t)(((raw_value - CAPACITIVE_SOIL_MOISTURE_SENSOR_MIN_VOLTAGE) / (float)(CAPACITIVE_SOIL_MOISTURE_SENSOR_MAX_VOLTAGE - CAPACITIVE_SOIL_MOISTURE_SENSOR_MIN_VOLTAGE)) * 100);
    return compensated < 0 ? 0 : (compensated > 100 ? 100 : compensated);
}

uint32_t capacitive_soil_moisture_sensor_read_compensated(capacitive_soil_moisture_sensor_t *sensor) {
    if (!sensor) {
        ESP_LOGE(TAG, "Sensor struct pointer is Null");
        return 0.0f;
    }

    uint32_t raw_value = capacitive_soil_moisture_sensor_read_value(sensor);
    return capacitive_soil_moisture_sensor_compensate(raw_value);
}