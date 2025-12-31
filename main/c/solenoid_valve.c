#include "esp_err.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "solenoid_valve.h"


#define PID_INTERVAL_S 5
#define VALVE_TASK_STACK_SIZE 4096

static float current_pwm = 0.0;
static gpio_num_t output_gpio;


static void vTaskSolenoidValvePwm(void * _) {
    const TickType_t ticks_on = current_pwm * ((PID_INTERVAL_S * 1000) / portTICK_PERIOD_MS);
    const TickType_t ticks_off = (1 - current_pwm) * ((PID_INTERVAL_S * 1000) / portTICK_PERIOD_MS);

    gpio_set_level(output_gpio, 1);
    vTaskDelay(ticks_on);

    gpio_set_level(output_gpio, 0);
    vTaskDelay(ticks_off);
}

esp_err_t solenoid_valve_init(gpio_num_t gpio_num) {
    esp_err_t err = gpio_set_direction(gpio_num, GPIO_MODE_OUTPUT);
    if (err != ESP_OK) return err;

    xTaskCreate(vTaskSolenoidValvePwm, "Solenoid Valve Pwm", VALVE_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

    return ESP_OK;
};


esp_err_t solenoid_valve_set_pwm(float value) {
    current_pwm = value;

    return ESP_OK;
};
