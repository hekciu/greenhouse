#include <memory.h>

#include "driver/gpio.h"
#include <rom/ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "hal/gpio_hal.h"


#include "dht22.h"


#define DHT22_TRANSMISSION_GPIO 18


static inline void _dht22_set_pin_high() {
    REG_WRITE(GPIO_OUT_W1TC_REG, DHT22_TRANSMISSION_GPIO);
}


static inline void _dht22_set_pin_low() {
    REG_WRITE(GPIO_OUT_W1TS_REG, DHT22_TRANSMISSION_GPIO);
}


esp_err_t dht22_initialize() {
    esp_err_t err;

    err = gpio_set_direction(DHT22_TRANSMISSION_GPIO, GPIO_MODE_INPUT_OUTPUT_OD);

    if (err != ESP_OK) return err;

    err = gpio_set_pull_mode(DHT22_TRANSMISSION_GPIO, GPIO_PULLUP_ONLY);

    if (err != ESP_OK) return err;

    return ESP_OK;
};


// https://cdn.sparkfun.com/assets/f/7/d/9/c/DHT22.pdf
esp_err_t dht22_read(dht22_data * const data) {
    portDISABLE_INTERRUPTS();
    vTaskSuspendAll();

    uint8_t output_data[sizeof(dht22_data)] = {0};

    _dht22_set_pin_low();
    ets_delay_us(1000);
    _dht22_set_pin_high();

    // create timer

    // infinite 40 * infinite loop waiting for states to change + some maximum time

    xTaskResumeAll();
    portENABLE_INTERRUPTS();

    memcpy(data, output_data, sizeof(dht22_data));

    return ESP_OK;
};


float dht22_get_RH(const dht22_data * const data) {
    return ((float)((data->integral_RH_data << 8) + data->decimal_RH_data) / 10.f);
};


float dht22_get_T(const dht22_data * const data) {
    return ((float)((data->integral_T_data << 8) + data->decimal_T_data) / 10.f);
};


int dht22_is_checksum_valid(const dht22_data * const data) {
    return 3;
};
