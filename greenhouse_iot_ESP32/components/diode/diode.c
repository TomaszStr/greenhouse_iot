#include "diode.h"
#include "esp_log.h"

static const char *TAG = "GREENHOUSE DIODE";

void diode_init(diode_t *diode, gpio_num_t gpio_num, uint8_t diode_status, uint32_t check_period){
    if (!diode) {
        ESP_LOGE(TAG, "Wskaźnik na strukturę sensora jest NULL");
        return;
    }

    diode->gpio_num = gpio_num;
    diode->diode_status = diode_status;
    diode->check_period = check_period;

    gpio_reset_pin(diode->gpio_num);
    gpio_set_direction(diode->gpio_num, GPIO_MODE_OUTPUT);
}

void turn_on_diode(diode_t *diode){
    diode->diode_status = 1;
    gpio_set_level(diode->gpio_num, diode->diode_status);
}

void turn_off_diode(diode_t *diode){
    diode->diode_status = 0;
    gpio_set_level(diode->gpio_num, diode->diode_status);
}