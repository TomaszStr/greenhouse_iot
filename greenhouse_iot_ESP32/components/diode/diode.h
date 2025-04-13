#ifndef DIODE_H
#define DIODE_H

#include "driver/gpio.h"

typedef struct {
    gpio_num_t gpio_num;
    uint8_t diode_status;
    uint32_t check_period;
} diode_t;

void diode_init(diode_t *diode, gpio_num_t gpio_num, uint8_t diode_status, uint32_t check_period);

void turn_on_diode(diode_t *diode);

void turn_off_diode(diode_t *diode);

#endif // DIODE_H