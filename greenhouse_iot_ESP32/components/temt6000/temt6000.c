#include "temt6000.h"
#include "esp_log.h"

static const char *TAG = "TEMT6000";

static uint32_t oversample_read(temt6000_t *sensor, uint16_t samples) {
    uint32_t total = 0;
    for (uint16_t i = 0; i < samples; i++) {
        total += adc1_get_raw(sensor->adc_channel);
    }
    total /= samples;
    uint32_t voltage = esp_adc_cal_raw_to_voltage(total, &(sensor->adc_chars));
    return voltage;
}

int temt6000_init(temt6000_t *sensor, adc_unit_t adc_unit, adc1_channel_t adc_channel, adc_atten_t adc_atten_db, adc_bits_width_t adc_bits_width, int default_vref) {
    if (!sensor) {
        ESP_LOGE(TAG, "TEMT6000 sensor struct pointer is Null");
        return -1;
    }
    
    esp_adc_cal_characteristics_t adc_chars = {0};
    esp_adc_cal_characterize(adc_unit, adc_atten_db, adc_bits_width, default_vref, &adc_chars);

    sensor->adc_channel = adc_channel;
    sensor->adc_chars = adc_chars;
    sensor->oversampling = TEMT6000_OVERSAMPLING_4X;
    sensor->last_raw_value = 0;

    esp_err_t ret = adc1_config_channel_atten(sensor->adc_channel, adc_atten_db);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Configuration error for TEMT6000 ADC: %s", esp_err_to_name(ret));
        return -2;
    }

    ESP_LOGI(TAG, "Sensor initiated successfully on channel: %d", sensor->adc_channel);
    return 0;
}

uint32_t temt6000_read_value(temt6000_t *sensor) {
    if (!sensor) {
        ESP_LOGE(TAG, "TEMT6000 sensor struct pointer is Null");
        return 0;
    }

    uint32_t value;
    switch (sensor->oversampling) {
        case TEMT6000_OVERSAMPLING_NONE:
            value = sensor->last_raw_value;
            break;
        case TEMT6000_OVERSAMPLING_2X:
        case TEMT6000_OVERSAMPLING_4X:
        case TEMT6000_OVERSAMPLING_8X:
        case TEMT6000_OVERSAMPLING_16X:
        case TEMT6000_OVERSAMPLING_32X:
        case TEMT6000_OVERSAMPLING_64X:
            value = oversample_read(sensor, sensor->oversampling);
            break;
        default:
            ESP_LOGW(TAG, "Incorrect oversampling level");
            value = adc1_get_raw(sensor->adc_channel);
            break;
    }

    sensor->last_raw_value = value;

    return value;
}

void temt6000_set_oversampling(temt6000_t *sensor, temt6000_oversampling_t oversampling) {
    if (!sensor) {
        ESP_LOGE(TAG, "TEMT6000 sensor struct pointer is Null");
        return;
    }

    if (oversampling == TEMT6000_OVERSAMPLING_NONE|| 
        oversampling == TEMT6000_OVERSAMPLING_2X  || 
        oversampling == TEMT6000_OVERSAMPLING_4X  || 
        oversampling == TEMT6000_OVERSAMPLING_8X  || 
        oversampling == TEMT6000_OVERSAMPLING_16X ||
        oversampling == TEMT6000_OVERSAMPLING_32X ||
        oversampling == TEMT6000_OVERSAMPLING_64X) {
        sensor->oversampling = oversampling;
        ESP_LOGI(TAG, "Oversampling set to: %dx", oversampling);
    } else {
        ESP_LOGW(TAG, "Incorrect oversampling level: %d", oversampling);
    }
}

uint32_t temt6000_compensate(uint32_t raw_value) {
    int32_t compensated = (int32_t)(((raw_value - TEMT6000_MIN_VOLTAGE) / (float)(TEMT6000_MAX_VOLTAGE - TEMT6000_MIN_VOLTAGE)) * 100);
    return compensated < 0 ? 0 : (compensated > 100 ? 100 : compensated);
}

uint32_t temt6000_read_compensated(temt6000_t *sensor) {
    if (!sensor) {
        ESP_LOGE(TAG, "TEMT6000 sensor struct pointer is Null");
        return 0;
    }

    uint32_t raw_value = temt6000_read_value(sensor);
    return temt6000_compensate(raw_value);
}