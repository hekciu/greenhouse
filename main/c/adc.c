#include "adc.h"

#include "esp_log.h"
#include "soc/adc_channel.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali_scheme.h"

#define CHOSEN_ADC_CHANNEL ADC_CHANNEL_0
#define ATTEN ADC_ATTEN_DB_12
#define BITWIDTH ADC_BITWIDTH_DEFAULT
#define UNIT ADC_UNIT_1

static const char* TAG = "ADC";

static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t cali_handle;
static int result = 0;


esp_err_t initialize_adc() {
    esp_err_t err;

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    err = adc_oneshot_new_unit(&init_config1, &adc1_handle);
    if (err != ESP_OK) return err;

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = BITWIDTH,
        .atten = ATTEN,
    };

    err = adc_oneshot_config_channel(adc1_handle, CHOSEN_ADC_CHANNEL, &config);
    if (err != ESP_OK) return err;

    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = UNIT,
        .atten = ATTEN,
        .bitwidth = BITWIDTH,
    };

    err = adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle);
    if (err != ESP_OK) return err;

    ESP_LOGI(TAG, "ADC initialization completed");

    return ESP_OK;
}


esp_err_t clear_adc() {
    esp_err_t err;

    err = adc_oneshot_del_unit(adc1_handle);
    if (err != ESP_OK) return err;

    err = adc_cali_delete_scheme_line_fitting(cali_handle);

    return ESP_OK;
}


esp_err_t adc_read(int* output) {
    esp_err_t err;

    err = adc_oneshot_get_calibrated_result(adc1_handle, cali_handle, CHOSEN_ADC_CHANNEL, &result);
    if (err != ESP_OK) return err;

    *output = result;
    
    ESP_LOGI(TAG, "ADC%d Channel[%d] Data: %d", ADC_UNIT_1 + 1, CHOSEN_ADC_CHANNEL, result);

    return ESP_OK;

}
