#ifndef PID_H
#define PID_H

#include <stdbool.h>

#include "esp_err.h"


typedef struct {
    uint32_t p;
    uint32_t i;
    uint32_t d;
    float last_error_value;
    double error_sum;
    bool enabled;
    float given_value;
    int interval_s;
} pid_handle;


uint8_t pid_calculate(pid_handle* handle, float current_value);
void pid_set_given_value(pid_handle* handle, float value);
float pid_get_given_value(pid_handle* handle);
void pid_set_P(pid_handle* handle, float P);
void pid_set_I(pid_handle* handle, float I);
void pid_set_D(pid_handle* handle, float D);
void pid_enable(pid_handle* handle, int _interval_s);
void pid_disable(pid_handle* handle);

#endif
