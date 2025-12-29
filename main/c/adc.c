#include "adc.h"

#include "soc/adc_channel.h"
#include "esp_adc/adc_oneshot.h"


static adc_oneshot_unit_handle_t adc1_handle;

esp_err_t initialize_adc() {
    esp_err_t err;

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    err = adc_oneshot_new_unit(&init_config1, &adc1_handle);
    if (err != ESP_OK) return err;

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    err = adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config);
    if (err != ESP_OK) return err;

    return ESP_OK;
}


esp_err_t clear_adc() {
    err = adc_oneshot_del_unit(adc1_handle);
    if (err != ESP_OK) return err;
}


esp_err_t adc_read() {
    err = adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN0, &adc_raw[0][0]);
    if (err != ESP_OK) return err;
    
    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN0, adc_raw[0][0]);

}
