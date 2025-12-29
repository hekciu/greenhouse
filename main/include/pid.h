#ifndef PID_H
#define PID_H

#include "esp_err.h"


#define PID_STEP_S 5

uint8_t pid_calculate(const float current_value);
void pid_set_given_value(const float value);
float pid_get_given_value();
void pid_set_P(const float P);
void pid_set_I(const float I);
void pid_set_D(const float D);
void pid_enable();
void pid_disable();

#endif
