#include "esp_err.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "solenoid_valve.h"


#define PID_INTERVAL_S 1
#define VALVE_TASK_STACK_SIZE 4096

static float current_pwm = 0.0;
static gpio_num_t output_gpio;
static const char* TAG = "SOLENOID VALVE";


static void vTaskSolenoidValvePwm(void * _) {

    while (1) {
        const TickType_t ticks_on = current_pwm * ((PID_INTERVAL_S * 1000) / portTICK_PERIOD_MS);
        const TickType_t ticks_off = (1 - current_pwm) * ((PID_INTERVAL_S * 1000) / portTICK_PERIOD_MS);

        ESP_LOGI(TAG, "high for: %ld, low for: %ld", ticks_on, ticks_off);

        if (ticks_on > 0) {
            gpio_set_level(output_gpio, 1);
            vTaskDelay(ticks_on);
        }

        if (ticks_off > 0) {
            gpio_set_level(output_gpio, 0);
            vTaskDelay(ticks_off);
        }
    }
}

esp_err_t solenoid_valve_init(gpio_num_t gpio_num) {
    esp_err_t err;

    output_gpio = gpio_num;

    err = gpio_set_direction(output_gpio, GPIO_MODE_OUTPUT);
    if (err != ESP_OK) return err;

    /*
    err = gpio_set_pull_mode(output_gpio, GPIO_PULLUP_PULLDOWN);
    if (err != ESP_OK) return err;
    */

    xTaskCreate(vTaskSolenoidValvePwm, "Solenoid Valve Pwm", VALVE_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, NULL);

    return ESP_OK;
};


esp_err_t solenoid_valve_set_pwm(float value) {
    current_pwm = value;

    return ESP_OK;
};

int solenoid_valve_get_pwm() {
    return (int)(100.0f * current_pwm);
}
