#include <stdbool.h>
#include "pid.h"



static float _calculate_derivative(float current_error, float last_error, float interval_s) {
    return (last_error - current_error) / (float)interval_s;
};


uint8_t pid_calculate(pid_handle* handle, float current_value) {
    if (!handle->enabled) return 0.0f;

    const float error = handle->given_value - current_value;

    handle->error_sum += error;

    const float p_component = handle->p * error;
    const float i_component = handle->i * handle->error_sum;
    const float d_component = handle->d
        * _calculate_derivative(error, handle->last_error_value, handle->interval_s);

    handle->last_error_value = error;

    float output = p_component + i_component + d_component;

    if (output > 100.0f) output = 100.0f;
    if (output < 0.0f) output = 0.0f;

    return (uint8_t)output;
};


void pid_set_given_value(pid_handle* handle, float value) {
    handle->given_value = value;
};


float pid_get_given_value(pid_handle* handle) {
    return handle->given_value;
};


void pid_set_P(pid_handle* handle, float P) {
    handle->p = P;
};


void pid_set_I(pid_handle* handle, float I) {
    handle->i = I;
};


void pid_set_D(pid_handle* handle, float D) {
    handle->d = D;
};


void pid_enable(pid_handle* handle, int _interval_s) {
    handle->enabled = true;
    handle->interval_s = _interval_s;
};


void pid_disable(pid_handle* handle) {
    handle->enabled = false;
};
