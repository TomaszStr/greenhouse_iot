#ifndef CAPACITIVE_SOIL_MOISTURE_SENSOR_H
#define CAPACITIVE_SOIL_MOISTURE_SENSOR_H

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <stdint.h>

/**
 * @brief Minimum and maximum voltage values in millivolts used for compensating raw readings to 0-100 scale
 */
#define CAPACITIVE_SOIL_MOISTURE_SENSOR_MAX_VOLTAGE 2250
#define CAPACITIVE_SOIL_MOISTURE_SENSOR_MIN_VOLTAGE 900

/**
 * @brief Typ wyliczeniowy dla oversamplingu.
 */
typedef enum {
    CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_NONE = 0,
    CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_2X   = 2,
    CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_4X   = 4,
    CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_8X   = 8,
    CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_16X  = 16,
    CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_32X  = 32,
    CAPACITIVE_SOIL_MOISTURE_SENSOR_OVERSAMPLING_64X  = 64,
} capacitive_soil_moisture_sensor_oversampling_t;

/**
 * @brief Struktura konfiguracji i stanu czujnika.
 */
typedef struct {
    adc1_channel_t adc_channel;      ///< Kanał ADC.
    esp_adc_cal_characteristics_t adc_chars;
    capacitive_soil_moisture_sensor_oversampling_t oversampling; ///< Poziom oversamplingu.
    uint32_t last_raw_value;         ///< Ostatnia surowa wartość.
} capacitive_soil_moisture_sensor_t;

/**
 * @brief Inicjalizuje czujnik na określonym pinie ADC.
 *
 * @param capacitive_soil_moisture_sensor Wskaźnik na strukturę capacitive_soil_moisture_sensora.
 * @param adc_channel Kanał ADC (np. ADC1_CHANNEL_0).
 * @return 0 jeśli sukces, kod błędu w przeciwnym wypadku.
 */
int capacitive_soil_moisture_sensor_init(capacitive_soil_moisture_sensor_t *capacitive_soil_moisture_sensor, adc_unit_t adc_unit, adc1_channel_t adc_channel, adc_atten_t adc_atten_db, adc_bits_width_t adc_bits_width, int default_vref);

/**
 * @brief Odczytuje wartość analogową z czujnika.
 *
 * @param capacitive_soil_moisture_sensor Wskaźnik na strukturę capacitive_soil_moisture_sensora.
 * @return Przetworzona wartość analogowa.
 */
uint32_t capacitive_soil_moisture_sensor_read_value(capacitive_soil_moisture_sensor_t *capacitive_soil_moisture_sensor);

/**
 * @brief Ustawia poziom oversamplingu dla odczytów.
 *
 * @param capacitive_soil_moisture_sensor Wskaźnik na strukturę capacitive_soil_moisture_sensora.
 * @param oversampling Poziom oversamplingu.
 */
void capacitive_soil_moisture_sensor_set_oversampling(capacitive_soil_moisture_sensor_t *capacitive_soil_moisture_sensor, capacitive_soil_moisture_sensor_oversampling_t oversampling);

/**
 * @brief Przekształca surową wartość ADC na wynik.
 *
 * @param raw_value Surowa wartość ADC.
 * @return Przekształcona wartość.
 */
uint32_t capacitive_soil_moisture_sensor_compensate(uint32_t raw_value);

/**
 * @brief Odczytuje i zwraca przekształconą wartość z czujnika.
 *
 * @param capacitive_soil_moisture_sensor Wskaźnik na strukturę capacitive_soil_moisture_sensora.
 * @return Przekształcona wartość.
 */
uint32_t capacitive_soil_moisture_sensor_read_compensated(capacitive_soil_moisture_sensor_t *capacitive_soil_moisture_sensor);

#endif // CAPACITIVE_SOIL_MOISTURE_SENSOR_H