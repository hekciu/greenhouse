#include <stdio.h>
#include "esp_wifi.h"
#include "esp_http_server.h"
#include <esp_wifi_types_generic.h>
#include "nvs_flash.h"


esp_err_t initialize_nvs();
esp_err_t initialize_access_point();
esp_err_t initialize_http_server(httpd_handle_t * handle);
