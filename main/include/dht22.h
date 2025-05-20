
#include "esp_err.h"


typedef struct {
    uint8_t integral_RH_data;
    uint8_t decimal_RH_data;
    uint8_t integral_T_data;
    uint8_t decimal_T_data;
    uint8_t checksum;
} dht22_data;


esp_err_t dht22_initialize();
esp_err_t dht22_read(dht22_data * const data);
float dht22_get_RH(const dht22_data * const data);
float dht22_get_T(const dht22_data * const data);
int dht22_is_checksum_valid(const dht22_data * const data);
