#ifndef SOLENOID_VALVE_H
#define SOLENOID_VALVE_H

#include "esp_err.h"
#include "driver/gpio.h"

esp_err_t solenoid_valve_init(gpio_num_t gpio_num);
esp_err_t solenoid_valve_set_pwm(float value);
int solenoid_valve_get_pwm();


#endif
