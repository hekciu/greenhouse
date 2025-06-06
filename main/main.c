#include <stdio.h>
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include <esp_wifi_types_generic.h>

#include "network.h"
#include "pwm.h"
#include "pid.h"
#include "utils.h"
#include "dht22.h"


#define QUERY_BUFFER_SIZE 2000
#define VALUE_PARAM_BUFFER_SIZE 4
#define MAIN_TASK_STACK_SIZE 4096
#define MAX_DEBUG_INFO_SIZE 400

enum GreenhouseError {
    G_ERR_NONE = 0,
    G_ERR_DHT22_FAILED = 1,
    G_ERR_PWM_FAILED = 2
};

static enum GreenhouseError last_error = G_ERR_NONE;
static float current_temperature = 0.0f;
static uint32_t time_since_last_measurement = 0;
static uint8_t current_pwm = 0;

static const char * TAG = "MAIN";


static const char resp_ok[] = "Ok";
static const char resp_bad_request[] = "Bad Request";
static const char resp_internal_server_error[] = "Internal Server Error";


static char query_buffer[QUERY_BUFFER_SIZE];
static char value_buffer[VALUE_PARAM_BUFFER_SIZE];


static esp_err_t main_endpoint_handler(httpd_req_t * req) {
    const char resp[] = "greenhouse panel will be here";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
};


static esp_err_t debug_endpoint_handler(httpd_req_t * req) {
    char resp[MAX_DEBUG_INFO_SIZE];

    snprintf(
        resp,
        MAX_DEBUG_INFO_SIZE - 1,
        "last error: %d\ncurrent temperature: %f\n given temperature: %f\ntime since last measurement: %ld\n s current power: %d percent\n",
        last_error,
        current_temperature,
        pid_get_given_value(),
        time_since_last_measurement,
        current_pwm
    );

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
};


static esp_err_t set_given_value_endpoint_handler(httpd_req_t * req) {
    esp_err_t err;

    err = httpd_req_get_url_query_str(req, query_buffer, QUERY_BUFFER_SIZE);

    if (err != ESP_OK) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, resp_bad_request, HTTPD_RESP_USE_STRLEN);
        return err;
    }

    err = httpd_query_key_value(query_buffer, "value", value_buffer, VALUE_PARAM_BUFFER_SIZE);

    if (err != ESP_OK) {
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, resp_internal_server_error, HTTPD_RESP_USE_STRLEN);
        return err;
    }

    float value = atof(value_buffer);

    pid_set_given_value(value);

    httpd_resp_send(req, resp_ok, HTTPD_RESP_USE_STRLEN);

    return err;
};


static esp_err_t set_pwm_endpoint_handler(httpd_req_t * req) {
    esp_err_t err;

    err = httpd_req_get_url_query_str(req, query_buffer, QUERY_BUFFER_SIZE);

    if (err != ESP_OK) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, resp_bad_request, HTTPD_RESP_USE_STRLEN);
        return err;
    }

    err = httpd_query_key_value(query_buffer, "value", value_buffer, VALUE_PARAM_BUFFER_SIZE);

    if (err != ESP_OK) {
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, resp_internal_server_error, HTTPD_RESP_USE_STRLEN);
        return err;
    }

    uint8_t value = atoi(value_buffer);

    err = set_pwm(value);

    if (err == ESP_OK) {
        httpd_resp_send(req, resp_ok, HTTPD_RESP_USE_STRLEN);
    } else {
        httpd_resp_set_status(req, "500");
        httpd_resp_send(req, resp_internal_server_error, HTTPD_RESP_USE_STRLEN);
    }

    return err;
};


static httpd_uri_t main_endpoint = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = main_endpoint_handler,
    .user_ctx = NULL
};


static httpd_uri_t set_given_value_endpoint = {
    .uri = "/api/temperature",
    .method = HTTP_POST,
    .handler = set_given_value_endpoint_handler,
    .user_ctx = NULL
};


static httpd_uri_t set_pwm_endpoint = {
    .uri = "/api/pwm",
    .method = HTTP_POST,
    .handler = set_pwm_endpoint_handler,
    .user_ctx = NULL
};


static httpd_uri_t debug_endpoint = {
    .uri = "/debug",
    .method = HTTP_GET,
    .handler = debug_endpoint_handler,
    .user_ctx = NULL
};


static void error_check(esp_err_t err) {
    if (err == ESP_OK) return;

    printf("an error has occured, details: %s\n", esp_err_to_name(err));
    exit(1);
}


static void vTaskRegulateTemperature(void * _) {
    dht22_data current_data = {0};
    esp_err_t err;

    while (1) {
        time_since_last_measurement += PID_STEP_S;
        err = dht22_read(&current_data);

        int checksum_valid = dht22_is_checksum_valid(&current_data);

        if (err != ESP_OK || checksum_valid != 0) {
            ESP_LOGE(TAG, "an error has occured while reading dht22 data");
            pid_disable();
            last_error = G_ERR_DHT22_FAILED;

            vTaskDelay((PID_STEP_S * 1000) / portTICK_PERIOD_MS);
            continue;
        } else {
            time_since_last_measurement = 0;
            pid_enable();
        }

        float temperature = dht22_get_T(&current_data);

        current_temperature = temperature;

        uint8_t heater_power_percent = pid_calculate(temperature);

        float current_given_value = pid_get_given_value();

        ESP_LOGI(TAG, "given temp: %f C, current temp: %f C, setting power to %d percent\n", current_given_value, temperature, heater_power_percent);

        err = set_pwm(heater_power_percent);

        if (err != ESP_OK) {
            ESP_LOGE(TAG, "error setting pwm\n");
            last_error = G_ERR_PWM_FAILED;
        } else {
            current_pwm = heater_power_percent;
            last_error = G_ERR_NONE;
        }

        vTaskDelay((PID_STEP_S * 1000) / portTICK_PERIOD_MS);
    }
};


void app_main(void) {
    const float default_given_value = 40.0f;

    error_check(dht22_initialize());

    pid_enable();
    pid_set_P(3);
    pid_set_I(0.001);
    pid_set_D(0.001);
    pid_set_given_value(default_given_value);

    error_check(initialize_nvs());
    error_check(initialize_access_point());

    httpd_handle_t handle;

    error_check(initialize_http_server(&handle));

    error_check(httpd_register_uri_handler(handle, &main_endpoint));

    error_check(httpd_register_uri_handler(handle, &set_pwm_endpoint));

    error_check(httpd_register_uri_handler(handle, &set_given_value_endpoint));

    error_check(httpd_register_uri_handler(handle, &debug_endpoint));

    error_check(initialize_pwm());

    error_check(set_pwm(0));

    xTaskCreate(vTaskRegulateTemperature, "Temperature Regulation", MAIN_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
}
