idf_component_register(SRCS "main.c" "c/network.c" "c/pwm.c" "c/utils.c"
                    PRIV_REQUIRES esp_wifi
                    PRIV_REQUIRES nvs_flash
                    PRIV_REQUIRES esp_http_server
                    PRIV_REQUIRES esp_driver_mcpwm
                    INCLUDE_DIRS "include")
