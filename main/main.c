#include <stdio.h>
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include <esp_wifi_types_generic.h>

#include "network.h"
#include "pwm.h"


static esp_err_t main_endpoint_handler(httpd_req_t * req) {
    const char resp[] = "greenhouse panel will be here";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
};


static httpd_uri_t main_endpoint = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = main_endpoint_handler,
    .user_ctx = NULL
};


static void error_check(esp_err_t err) {
    if (err == ESP_OK) return;

    printf("an error has occured, details: %s\n", esp_err_to_name(err));
    exit(1);
}



void app_main(void) {
    esp_err_t err;

    error_check(initialize_nvs());
    error_check(initialize_access_point());

    httpd_handle_t handle;

    error_check(initialize_http_server(&handle));

    error_check(httpd_register_uri_handler(handle, &main_endpoint));

    error_check(initialize_pwm());
}
