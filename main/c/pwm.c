#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"

#include "esp_log.h"

static const char * TAG = "PWM";


#define RESOLUTION 1000000
#define PERIOD_TICKS 20000
#define OUTPUT_GPIO 0


static inline uint32_t percent_to_compare(uint8_t input) {
    return (float)((float)input / (float)100) * PERIOD_TICKS;
};



static mcpwm_cmpr_handle_t comparator = NULL;


// https://github.com/espressif/esp-idf/blob/v5.4.1/examples/peripherals/mcpwm/mcpwm_servo_control/main/mcpwm_servo_control_example_main.c
esp_err_t initialize_pwm() {
    esp_err_t err;

    // MCPWM Timer
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = RESOLUTION,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = PERIOD_TICKS,
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

    err = mcpwm_operator_connect_timer(oper, timer_handler);

    if (err != ESP_OK) {
        return err;
    }

    // MCPWM Comparator
    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };

    err = mcpwm_new_comparator(oper, &comparator_config, &comparator);

    if (err != ESP_OK) {
        return err;
    }

    // MCPWM Generator
    mcpwm_gen_handle_t generator = NULL;
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = OUTPUT_GPIO,
    };

    err = mcpwm_new_generator(oper, &generator_config, &generator);

    if (err != ESP_OK) {
        return err;
    }

    err = mcpwm_comparator_set_compare_value(comparator, percent_to_compare(0));

    if (err != ESP_OK) {
        return err;
    }

    err = mcpwm_generator_set_action_on_timer_event(generator,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH));

    if (err != ESP_OK) {
        return err;
    }

    err = mcpwm_generator_set_action_on_compare_event(generator,
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW));

    if (err != ESP_OK) {
        return err;
    }

    err = mcpwm_timer_enable(timer_handler);

    if (err != ESP_OK) {
        return err;
    }

    err = mcpwm_timer_start_stop(timer_handler, MCPWM_TIMER_START_NO_STOP);

    return err;
}


void * use_pwm_task(void * data) {
    return NULL;
}


esp_err_t set_pwm(uint8_t percent) {
    percent = percent > 100 ? 100 : percent;

    ESP_LOGI(TAG, "setting power to %d percent", percent);

    return mcpwm_comparator_set_compare_value(comparator, percent_to_compare(percent));
}
