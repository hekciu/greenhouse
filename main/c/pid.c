#include <stdbool.h>
#include "pid.h"


static uint32_t pid_P = 0;
static uint32_t pid_I = 0;
static uint32_t pid_D = 0;

static float pid_last_error_value = 0.0f;
static double pid_error_sum = 0.0f;

static bool pid_enabled = false;
static float pid_given_value = 0.0f;


static float _calculate_derivative(const float current_error, const float last_error) {
    return (last_error - current_error) / (float)PID_STEP_S;
};


uint8_t pid_calculate(const float current_value) {
    if (!pid_enabled) return 0.0f;

    const float error = pid_given_value - current_value;

    pid_error_sum += error;

    const float p_component = pid_P * error;
    const float i_component = pid_I * pid_error_sum;
    const float d_component = pid_D * _calculate_derivative(error, pid_last_error_value);

    pid_last_error_value = error;

    float output = p_component + i_component + d_component;

    if (output > 100.0f) output = 100.0f;
    if (output < 0.0f) output = 0.0f;

    return (uint8_t)output;
};


void pid_set_given_value(const float value) {
    pid_given_value = value;
};


void pid_set_P(const float P) {
    pid_P = P;
};


void pid_set_I(const float I) {
    pid_I = I;
};


void pid_set_D(const float D) {
    pid_D = D;
};


void pid_enable() {
    pid_enabled = true;
};


void pid_disable() {
    pid_enabled = false;
};
