idf_component_register(SRCS "main.c" "c/pid.c" "c/dht22.c" "c/network.c" "c/pwm.c" "c/utils.c"
                    PRIV_REQUIRES esp_wifi
                    PRIV_REQUIRES nvs_flash
                    PRIV_REQUIRES esp_http_server
                    PRIV_REQUIRES esp_driver_mcpwm
                    PRIV_REQUIRES esp_driver_gpio
                    INCLUDE_DIRS "include")
