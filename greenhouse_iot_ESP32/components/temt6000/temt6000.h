#ifndef TEMT6000_H
#define TEMT6000_H

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <stdint.h>

/**
 * @brief Minimum and maximum voltage values in millivolts used for compensating raw readings to 0-100 scale
 */
#define TEMT6000_MAX_VOLTAGE 3160
#define TEMT6000_MIN_VOLTAGE 142

/**
 * @brief Typ wyliczeniowy dla oversamplingu.
 */
typedef enum {
    TEMT6000_OVERSAMPLING_NONE = 0,
    TEMT6000_OVERSAMPLING_2X   = 2,
    TEMT6000_OVERSAMPLING_4X   = 4,
    TEMT6000_OVERSAMPLING_8X   = 8,
    TEMT6000_OVERSAMPLING_16X  = 16,
    TEMT6000_OVERSAMPLING_32X  = 32,
    TEMT6000_OVERSAMPLING_64X  = 64
} temt6000_oversampling_t;

/**
 * @brief Struktura konfiguracji i stanu czujnika.
 */
typedef struct {
    adc1_channel_t adc_channel;      ///< Kanał ADC.
    esp_adc_cal_characteristics_t adc_chars;
    temt6000_oversampling_t oversampling; ///< Poziom oversamplingu.
    uint32_t last_raw_value;         ///< Ostatnia surowa wartość.
} temt6000_t;

/**
 * @brief Inicjalizuje czujnik na określonym pinie ADC.
 *
 * @param temt6000 Wskaźnik na strukturę temt6000a.
 * @param adc_channel Kanał ADC (np. ADC1_CHANNEL_0).
 * @return 0 jeśli sukces, kod błędu w przeciwnym wypadku.
 */
int temt6000_init(temt6000_t *sensor, adc_unit_t adc_unit, adc1_channel_t adc_channel, adc_atten_t adc_atten_db, adc_bits_width_t adc_bits_width, int default_vref);

/**
 * @brief Odczytuje wartość analogową z czujnika.
 *
 * @param temt6000 Wskaźnik na strukturę temt6000a.
 * @return Przetworzona wartość analogowa.
 */
uint32_t temt6000_read_value(temt6000_t *temt6000);

/**
 * @brief Ustawia poziom oversamplingu dla odczytów.
 *
 * @param temt6000 Wskaźnik na strukturę temt6000a.
 * @param oversampling Poziom oversamplingu.
 */
void temt6000_set_oversampling(temt6000_t *temt6000, temt6000_oversampling_t oversampling);

/**
 * @brief Przekształca surową wartość ADC na wynik.
 *
 * @param raw_value Surowa wartość ADC.
 * @return Przekształcona wartość.
 */
uint32_t temt6000_compensate(uint32_t raw_value);

/**
 * @brief Odczytuje i zwraca przekształconą wartość z czujnika.
 *
 * @param temt6000 Wskaźnik na strukturę temt6000a.
 * @return Przekształcona wartość.
 */
uint32_t temt6000_read_compensated(temt6000_t *temt6000);

#endif // TEMT6000_H