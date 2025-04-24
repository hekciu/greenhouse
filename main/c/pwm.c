#include "driver/mcpwm_timer.h"


static inline uint32_t percent_to_compare(uint32_t input) {
    return input * 
};


// https://github.com/espressif/esp-idf/blob/v5.4.1/examples/peripherals/mcpwm/mcpwm_servo_control/main/mcpwm_servo_control_example_main.c
esp_err_t initialize_pwm() {
    esp_err_t err;

    // MCPWM Timer
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = 20000,
    };

    mcpwm_timer_handle_t timer_handler = NULL;

    err = mcpwm_new_timer(&timer_config, &timer_handler);

    if (err != ESP_OK) {
        return err;
    }

    // MCPWM Operators
    mcpwm_oper_handle_t oper = NULL;
    mcpwm_operator_config_t operator_config = {
        .group_id = 0,
    };

    err = mcpwm_new_operator(&operator_config, &oper);

    if (err != ESP_OK) {
        return err;
    }

    err = mcpwm_operator_connect_timer(oper, timer);

    if (err != ESP_OK) {
        return err;
    }

    // MCPWM Comparator
    mcpwm_cmpr_handle_t comparator = NULL;
    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };

    err = mcpwm_new_comparator(oper, &generator_config, &comparator);

    if (err != ESP_OK) {
        return err;
    }

    // MCPWM Generator
    mcpwm_gen_handle_t generator = NULL;
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = OUTPUT_GPIO,
    };

    if (err != ESP_OK) {
        return err;
    }

    mcpwm_comparator_set_compare_value(comparator, percent_to_compare(0));
    return err;
}


void * use_pwm_task(void * data) {
    return NULL;
}


esp_err_t set_pwm(uint8_t percent) {
    return ESP_OK;
}
