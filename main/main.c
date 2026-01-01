#include <stdio.h>
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_wifi_types_generic.h"

#include "storage.h"
#include "network.h"
#include "pwm.h"
#include "pid.h"
#include "utils.h"
#include "dht22.h"
#include "adc.h"
#include "solenoid_valve.h"


#include "../../index.c"


#define TEMPERATURE_INTERVAL_S 5
#define HUMIDITY_INTERVAL_S 5
#define QUERY_BUFFER_SIZE 2000
#define VALUE_PARAM_BUFFER_SIZE 4
#define MAIN_TASK_STACK_SIZE 4096
#define MAX_DEBUG_INFO_SIZE 400
#define SOLENOID_VALVE_GPIO GPIO_NUM_23


enum GreenhouseError {
    G_ERR_NONE = 0,
    G_ERR_DHT22_FAILED = 1,
    G_ERR_PWM_FAILED = 2,
    G_ERR_ADC_FAILED = 3
};

static enum GreenhouseError last_error = G_ERR_NONE;

static float current_temperature = 0.0f;
static uint32_t time_since_last_temperature_measurement = 0;
static uint8_t current_pwm = 0;

static int current_humidity = 0.0f;
static uint32_t time_since_last_humidity_measurement = 0;
static float current_humidity_pwm = 0;

static pid_handle pid_temperature = { 0 };
static pid_handle pid_humidity = { 0 };

static storage_handle storage_temperature = { 0 };
static storage_handle storage_humidity = { 0 };

static const char * TAG = "MAIN";


static const char resp_ok[] = "Ok";
static const char resp_bad_request[] = "Bad Request";
static const char resp_internal_server_error[] = "Internal Server Error";


static char query_buffer[QUERY_BUFFER_SIZE];
static char value_buffer[VALUE_PARAM_BUFFER_SIZE];


static esp_err_t main_endpoint_handler(httpd_req_t * req) {
    httpd_resp_send(req, index_page, index_page_length);
    return ESP_OK;
};


static esp_err_t info_endpoint_handler(httpd_req_t * req) {
    char resp[MAX_DEBUG_INFO_SIZE];

    snprintf(
        resp,
        MAX_DEBUG_INFO_SIZE - 1,
        "last error: %d<br/>current temperature: %f<br/> given temperature: %f<br/><br/>current humidity: %f<br/> given humidity: %f<br/>time since last measurement: %ld s<br/> current power: %d percent\n",
        last_error,
        current_temperature,
        pid_get_given_value(&pid_temperature),
        (float)current_humidity,
        pid_get_given_value(&pid_humidity),
        time_since_last_temperature_measurement,
        current_pwm
    );

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
};


static esp_err_t get_temperature_measurements_endpoint_handler(httpd_req_t * req) {
    char * resp = NULL;

    storage_get_json_data(&storage_temperature, &resp);

    if (resp != NULL) {
        httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
        free(resp);
        return ESP_OK;
    } else {
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, resp_bad_request, HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
};


static esp_err_t get_humidity_measurements_endpoint_handler(httpd_req_t * req) {
    char * resp = NULL;

    storage_get_json_data(&storage_humidity, &resp);

    if (resp != NULL) {
        httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
        free(resp);
        return ESP_OK;
    } else {
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, resp_bad_request, HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
};


static esp_err_t set_given_temperature_value_endpoint_handler(httpd_req_t * req) {
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

    pid_set_given_value(&pid_temperature, value);

    httpd_resp_send(req, resp_ok, HTTPD_RESP_USE_STRLEN);

    return err;
};


static esp_err_t set_given_humidity_value_endpoint_handler(httpd_req_t * req) {
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

    pid_set_given_value(&pid_humidity, value);

    httpd_resp_send(req, resp_ok, HTTPD_RESP_USE_STRLEN);

    return err;
};


static httpd_uri_t main_endpoint = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = main_endpoint_handler,
    .user_ctx = NULL
};


static httpd_uri_t set_given_temperature_value_endpoint = {
    .uri = "/api/temperature",
    .method = HTTP_POST,
    .handler = set_given_temperature_value_endpoint_handler,
    .user_ctx = NULL
};


static httpd_uri_t set_given_humidity_value_endpoint = {
    .uri = "/api/humidity",
    .method = HTTP_POST,
    .handler = set_given_humidity_value_endpoint_handler,
    .user_ctx = NULL
};


static httpd_uri_t get_temperature_measurements_endpoint = {
    .uri = "/api/temperature",
    .method = HTTP_GET,
    .handler = get_temperature_measurements_endpoint_handler,
    .user_ctx = NULL
};


static httpd_uri_t get_humidity_measurements_endpoint = {
    .uri = "/api/humidity",
    .method = HTTP_GET,
    .handler = get_humidity_measurements_endpoint_handler,
    .user_ctx = NULL
};


static httpd_uri_t info_endpoint = {
    .uri = "/api/info",
    .method = HTTP_GET,
    .handler = info_endpoint_handler,
    .user_ctx = NULL
};


static void error_check(esp_err_t err) {
    if (err == ESP_OK) return;

    ESP_LOGE(TAG, "an error has occured, details: %s\n", esp_err_to_name(err));
    exit(1);
}


static void vTaskRegulateTemperature(void * _) {
    dht22_data current_data = {0};
    esp_err_t err;

    while (1) {
        time_since_last_temperature_measurement += TEMPERATURE_INTERVAL_S;
        err = dht22_read(&current_data);

        int checksum_valid = dht22_is_checksum_valid(&current_data);

        if (err != ESP_OK || checksum_valid != 0) {
            ESP_LOGE(TAG, "an error has occured while reading dht22 data");
            pid_disable(&pid_temperature);
            last_error = G_ERR_DHT22_FAILED;

            vTaskDelay((TEMPERATURE_INTERVAL_S * 1000) / portTICK_PERIOD_MS);
            continue;
        }

        time_since_last_temperature_measurement = 0;
        pid_enable(&pid_temperature, TEMPERATURE_INTERVAL_S);

        float temperature = dht22_get_T(&current_data);

        storage_add_measurement(&storage_temperature, temperature);

        current_temperature = temperature;

        uint8_t heater_power_percent = pid_calculate(&pid_temperature, temperature);

        float current_given_value = pid_get_given_value(&pid_temperature);

        ESP_LOGI(TAG, "given temp: %f C, current temp: %f C, setting power to %d percent\n", current_given_value, temperature, heater_power_percent);

        err = set_pwm(heater_power_percent);

        if (err != ESP_OK) {
            ESP_LOGE(TAG, "error setting pwm\n");
            last_error = G_ERR_PWM_FAILED;
        } else {
            current_pwm = heater_power_percent;
            last_error = G_ERR_NONE;
        }

        vTaskDelay((TEMPERATURE_INTERVAL_S * 1000) / portTICK_PERIOD_MS);
    }
};

static void vTaskRegulateSoilHumidity(void * _) {
    esp_err_t err;

    while (1) {
        int humidity = 0;

        err = adc_read(&humidity);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "error adc read\n");
            last_error = G_ERR_ADC_FAILED;
        }

        storage_add_measurement(&storage_humidity, humidity);

        current_humidity = humidity;

        uint8_t valve_open_percent = pid_calculate(&pid_humidity, humidity);

        float current_given_value = pid_get_given_value(&pid_humidity);

        ESP_LOGI(TAG, "given humidity: %f mV, current humidity: %d mV, valve open %d percent\n", current_given_value, humidity, valve_open_percent);

        solenoid_valve_set_pwm(((float)valve_open_percent) / 100.0f);

        vTaskDelay((HUMIDITY_INTERVAL_S * 1000) / portTICK_PERIOD_MS);
    }
};


void app_main(void) {
    error_check(initialize_adc());

    error_check(dht22_initialize());

    pid_enable(&pid_temperature, TEMPERATURE_INTERVAL_S);
    pid_set_P(&pid_temperature, 3);
    pid_set_I(&pid_temperature, 0.001);
    pid_set_D(&pid_temperature, 0.001);
    pid_set_given_value(&pid_temperature, 40.0f);

    pid_enable(&pid_humidity, HUMIDITY_INTERVAL_S);
    pid_set_P(&pid_humidity, 3);
    pid_set_I(&pid_humidity, 0.001);
    pid_set_D(&pid_humidity, 0.001);
    pid_set_given_value(&pid_humidity, 1000.0f); // mV

    error_check(initialize_nvs());
    error_check(initialize_access_point());

    httpd_handle_t handle;

    error_check(initialize_http_server(&handle));

    error_check(httpd_register_uri_handler(handle, &main_endpoint));

    error_check(httpd_register_uri_handler(handle, &set_given_temperature_value_endpoint));
    error_check(httpd_register_uri_handler(handle, &set_given_humidity_value_endpoint));

    error_check(httpd_register_uri_handler(handle, &get_temperature_measurements_endpoint));
    error_check(httpd_register_uri_handler(handle, &get_humidity_measurements_endpoint));

    error_check(httpd_register_uri_handler(handle, &info_endpoint));

    error_check(initialize_pwm());

    error_check(set_pwm(0));

    error_check(solenoid_valve_init(SOLENOID_VALVE_GPIO));

    error_check(solenoid_valve_set_pwm(0));

    /* Temperature */
    xTaskCreate(vTaskRegulateTemperature, "Temperature Regulation", MAIN_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

    /* Soil humidity */
    xTaskCreate(vTaskRegulateSoilHumidity, "Soil Humidity Regulation", MAIN_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
}
