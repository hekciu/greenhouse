#include <stdio.h>
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include <esp_wifi_types_generic.h>

#include "network.h"
#include "pwm.h"
#include "utils.h"
#include "dht22.h"


#define QUERY_BUFFER_SIZE 2000
#define VALUE_PARAM_BUFFER_SIZE 4


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

    // uint8_t value = str_to_first_char_ascii(value_buffer);
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


static httpd_uri_t set_pwm_enpoint = {
    .uri = "/api/pwm",
    .method = HTTP_POST,
    .handler = set_pwm_endpoint_handler,
    .user_ctx = NULL
};


static void error_check(esp_err_t err) {
    if (err == ESP_OK) return;

    printf("an error has occured, details: %s\n", esp_err_to_name(err));
    exit(1);
}



void app_main(void) {
    error_check(dht22_initialize());

    dht22_data test_data = {0};
    dht22_read(&test_data);

    printf(
        "temperature: %d relative humidity: %d checksum: %d checksum valid: %d\n",
        dht22_get_RH(&test_data),
        dht22_get_T(&test_data),
        test_data.checksum,
        dht22_is_checksum_valid(&test_data)
    );

    /*
    error_check(initialize_nvs());
    error_check(initialize_access_point());

    httpd_handle_t handle;

    error_check(initialize_http_server(&handle));

    error_check(httpd_register_uri_handler(handle, &main_endpoint));

    error_check(httpd_register_uri_handler(handle, &set_pwm_enpoint));

    error_check(initialize_pwm());

    error_check(set_pwm(50));
    */
}
