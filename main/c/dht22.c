#include "driver/gpio.h"

#include "dht22.h"


#define DHT22_TRANSMISSION_GPIO 18


esp_err_t dht22_initialize() {
    esp_err_t err;

    err = gpio_set_direction(DHT22_TRANSMISSION_GPIO, GPIO_MODE_INPUT_OUTPUT_OD);

    if (err != ESP_OK) return err;

    err = gpio_set_pull_mode(DHT22_TRANSMISSION_GPIO, GPIO_PULLUP_ONLY);

    if (err != ESP_OK) return err;

    return ESP_OK;
};


esp_err_t dht22_read(dht22_data * const data) {
    return ESP_OK;
};


uint16_t dht22_get_RH(const dht22_data * const data) {
    return 69;
};


uint16_t dht22_get_T(const dht22_data * const data) {
    return 420;
};


int dht22_is_checksum_valid(const dht22_data * const data) {
    return 3;
};
