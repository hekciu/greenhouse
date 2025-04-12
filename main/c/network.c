#include <stdio.h>
#include "esp_wifi.h"
#include "esp_http_server.h"
#include <esp_wifi_types_generic.h>
#include "nvs_flash.h"

#define PORT 2317

esp_err_t initialize_nvs() {
    esp_err_t err;

    err = nvs_flash_init();
    if (err != ESP_OK) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }

    return err;
}


esp_err_t initialize_access_point() {
    esp_err_t err;

	err = esp_netif_init();

    if (err != ESP_OK) {
        return err;
    }

	err = esp_event_loop_create_default();

    if (err != ESP_OK) {
        return err;
    }

    /*
        this one uses assert under the hood, program could possibly terminate
    */
	esp_netif_create_default_wifi_ap();

    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT(); 

    err = esp_wifi_init(&wifi_config);

    if (err != ESP_OK) {
        return err;
    }

    err = esp_wifi_set_mode(WIFI_MODE_AP);

    if (err != ESP_OK) {
        return err;
    }

    wifi_ap_config_t access_point_config = {
        .ssid = "Greenhouse",
        .password = "haslo123",
        .max_connection = 1,
        .channel = 1,
        .authmode = WIFI_AUTH_WPA2_PSK,
        .ssid_hidden = 0,
        .beacon_interval = 500,
        .csa_count = 3,
        .dtim_period = 1,
        .pairwise_cipher = WIFI_CIPHER_TYPE_WEP40,
        .ftm_responder = false,
        .pmf_cfg = {
            .required = false,
        },
    };

    wifi_config_t wifi_access_point_config = {
        .ap = access_point_config
    };

    err = esp_wifi_set_config(WIFI_IF_AP, &wifi_access_point_config);

    if (err != ESP_OK) {
        return err;
    }

    err = esp_wifi_start();

    return err;
}


esp_err_t initialize_http_server(httpd_handle_t * handle) {
    esp_err_t err;

    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();

    server_config.server_port = PORT;

    err = httpd_start(handle, &server_config);

    return err;
}

