#include "capacitive_soil_moisture_sensor.h"
#include "esp_log.h"

static const char *TAG = "CAPACITIVE_SOIL_MOISTURE_SENSOR";

/**
 * @brief Funkcja pomocnicza do wykonania oversamplingu.
 *
 * @param sensor Wskaźnik na strukturę sensora.
 * @param samples Ilość próbek do zsumowania.
 * @return Zsumowana wartość z ADC.
 */
static uint32_t oversample_read(capacitive_soil_moisture_sensor_t *sensor, uint16_t samples) {
    uint32_t total = 0;
    for (uint16_t i = 0; i < samples; i++) {
        total += adc1_get_raw(sensor->adc_channel);
    }
    total /= samples;
    uint32_t voltage = esp_adc_cal_raw_to_voltage(total, &(sensor->adc_chars));
    // ESP_LOGI(TAG, "Odczytana wartość: %ld[bit], Odczytana wartość: %ld[mV]", total, voltage);
    return voltage;
}

int capacitive_soil_moisture_sensor_init(capacitive_soil_moisture_sensor_t *sensor, adc_unit_t adc_unit, adc1_channel_t adc_channel, adc_atten_t adc_atten_db, adc_bits_width_t adc_bits_width, int default_vref) {
    if (!sensor) {
        ESP_LOGE(TAG, "Wskaźnik na strukturę sensora jest NULL");
        return -1;
    }

    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_characterize(adc_unit, adc_atten_db, adc_bits_width, default_vref, &adc_chars);

    // Przypisz kanał ADC
    sensor->adc_channel = adc_channel;
    sensor->adc_chars = adc_chars;
    sensor->oversampling = CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_4X;
    sensor->last_raw_value = 0;

    // // Konfiguracja ADC1
    // esp_err_t ret = adc1_config_width(ADC_WIDTH_BIT_12);
    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "Błąd konfiguracji szerokości ADC: %s", esp_err_to_name(ret));
    //     return -1;
    // }

    esp_err_t ret = adc1_config_channel_atten(sensor->adc_channel, adc_atten_db);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Błąd konfiguracji tłumienia ADC: %s", esp_err_to_name(ret));
        return -2;
    }

    ESP_LOGI(TAG, "Inicjalizacja czujnika zakończona sukcesem na kanale %d", sensor->adc_channel);
    return 0;
}

uint32_t capacitive_soil_moisture_sensor_read_value(capacitive_soil_moisture_sensor_t *sensor) {
    if (!sensor) {
        ESP_LOGE(TAG, "Wskaźnik na strukturę sensora jest NULL");
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
            ESP_LOGW(TAG, "Nieprawidłowy poziom oversamplingu, użycie domyślnego odczytu");
            value = adc1_get_raw(sensor->adc_channel);
            break;
    }

    sensor->last_raw_value = value;
    return value;
}

void capacitive_soil_moisture_sensor_set_oversampling(capacitive_soil_moisture_sensor_t *sensor, capacitive_soil_moisture_sensor_oversampling_t oversampling) {
    if (!sensor) {
        ESP_LOGE(TAG, "Wskaźnik na strukturę sensora jest NULL");
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
        ESP_LOGI(TAG, "Ustawiono oversampling na: %dx", oversampling);
    } else {
        ESP_LOGW(TAG, "Nieprawidłowy poziom oversamplingu: %d", oversampling);
    }
}

uint32_t capacitive_soil_moisture_sensor_compensate(uint32_t raw_value) {
    int32_t compensated = 100 - (int32_t)(((raw_value - CAPACITIVE_SOIL_MOISTURE_SENSOR_MIN_VOLTAGE) / (float)(CAPACITIVE_SOIL_MOISTURE_SENSOR_MAX_VOLTAGE - CAPACITIVE_SOIL_MOISTURE_SENSOR_MIN_VOLTAGE)) * 100);
    return compensated < 0 ? 0 : (compensated > 100 ? 100 : compensated);
}

uint32_t capacitive_soil_moisture_sensor_read_compensated(capacitive_soil_moisture_sensor_t *sensor) {
    if (!sensor) {
        ESP_LOGE(TAG, "Wskaźnik na strukturę sensora jest NULL");
        return 0.0f;
    }

    uint32_t raw_value = capacitive_soil_moisture_sensor_read_value(sensor);
    return capacitive_soil_moisture_sensor_compensate(raw_value);
}