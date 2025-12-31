idf_component_register(SRCS "main.c" "c/pid.c" "c/dht22.c" "c/network.c" "c/pwm.c" "c/utils.c" "c/storage.c" "c/adc.c" "c/solenoid_valve.c"
                    PRIV_REQUIRES esp_wifi
                    PRIV_REQUIRES nvs_flash
                    PRIV_REQUIRES esp_http_server
                    PRIV_REQUIRES esp_driver_mcpwm
                    PRIV_REQUIRES esp_driver_gpio
                    PRIV_REQUIRES esp_adc
                    INCLUDE_DIRS "include")
