idf_component_register(SRCS "main.c" "c/network.c"
                    PRIV_REQUIRES esp_wifi
                    PRIV_REQUIRES nvs_flash
                    PRIV_REQUIRES esp_http_server
                    INCLUDE_DIRS "")
