/*
    Reference:
    - https://cdn.sparkfun.com/assets/f/7/d/9/c/DHT22.pdf
    - https://cdn-shop.adafruit.com/product-files/3269/esp32_technical_reference_manual_en_0.pdf
    - https://github.com/gosouth/DHT22
*/


#include <memory.h>

#include "driver/gpio.h"
#include <rom/ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "hal/gpio_hal.h"
#include "hal/gpio_types.h"


#include "dht22.h"

#define DHT22_COUNTER_RESOLUTION 1000000
#define DHT22_TRANSMISSION_PIN GPIO_NUM_0

const char * TAG = "DHT22";


static inline uint8_t _dht22_wait_for_state(bool state, uint32_t timeout) {
    uint8_t us_elapsed = 0;

    // gpio_get_level(DHT22_TRANSMISSION_PIN) != state

    while (((REG_READ(GPIO_IN_REG) >> DHT22_TRANSMISSION_PIN) & 1ULL) != state) {
        if (us_elapsed > timeout) break;

        ets_delay_us(1);

        us_elapsed++;
    }

    return us_elapsed;
}


esp_err_t dht22_initialize() {
    esp_err_t err;

    err = gpio_set_pull_mode(DHT22_TRANSMISSION_PIN, GPIO_PULLUP_ONLY);

    return err;
};


static inline int _dht22_try_getting_data(uint8_t * output_data) {
    uint8_t current_elapsed;

    gpio_set_direction(DHT22_TRANSMISSION_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(DHT22_TRANSMISSION_PIN, 0);

    ets_delay_us(1500);

    gpio_set_direction(DHT22_TRANSMISSION_PIN, GPIO_MODE_INPUT);

    current_elapsed = _dht22_wait_for_state(0, 40);

    if (current_elapsed < 20 || current_elapsed > 40) {
        return 1;
    }

    // 1. started initial 80 ms low level

    current_elapsed = _dht22_wait_for_state(1, 80);

    if (current_elapsed < 60 || current_elapsed > 80) {
        return 2;
    }

    // 2. ended initial 80 ms low level and started 80ms high level

    current_elapsed = _dht22_wait_for_state(0, 80);

    if (current_elapsed < 60 || current_elapsed > 80) {
        return 3;
    }

    // 3. ended initial 80 ms high level and wait for data

    for (uint8_t i = 0; i < sizeof(dht22_data); i++) {
        // 4. every data starts with 50ms of low level

        *(output_data + i) = 0;

        for (uint8_t j = 0; j < 8; j++) {
            current_elapsed = _dht22_wait_for_state(1, 55);

            if (current_elapsed < 36 || current_elapsed > 55) {
                return 4;
            }

            current_elapsed = _dht22_wait_for_state(0, 75);

            if (current_elapsed > 12 && current_elapsed < 30) {
                // we got 0 so we add nothing
            } else if (current_elapsed > 50 && current_elapsed < 74) {
                // we got 1
                *(output_data + i) += (1 << (7 - j));
            } else {
                return 5;
            }
        }
    }

    return 0;
};


esp_err_t dht22_read(dht22_data * const data) {
    esp_err_t err;

    uint8_t output_data[sizeof(dht22_data)] = {0};

    portDISABLE_INTERRUPTS();
    vTaskSuspendAll();

    int success = _dht22_try_getting_data(output_data);

    xTaskResumeAll();
    portENABLE_INTERRUPTS();

    if (success == 0) {
        memcpy(data, output_data, sizeof(dht22_data));
        ESP_LOGI(TAG, "successfully read dht22 data\n");
        return ESP_OK;
    }

    ESP_LOGE(TAG, "reading dht22 data errored at stage %d\n", success);

    return ESP_FAIL;
};


float dht22_get_RH(const dht22_data * const data) {
    return ((float)((data->integral_RH_data << 8) + data->decimal_RH_data) / 10.f);
};


float dht22_get_T(const dht22_data * const data) {
    return ((float)((data->integral_T_data << 8) + data->decimal_T_data) / 10.f);
};


int dht22_is_checksum_valid(const dht22_data * const data) {
    const uint8_t sum = data->integral_RH_data + data->decimal_RH_data + data->integral_T_data + data->decimal_T_data;

    return data->checksum - sum;
};
