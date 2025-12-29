#ifndef ADC_H
#define ADC_H

#include "esp_err.h"

esp_err_t initialize_adc();
esp_err_t clear_adc();
esp_err_t adc_read();

#endif
