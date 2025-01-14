#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

const static char *TAG = "ADC";

/* ---------------------------------------- *
 *                ADC General Macros
 * ---------------------------------------- */
// ADC1 Channels
#define EXAMPLE_ADC1_CHAN0 ADC_CHANNEL_4
#define EXAMPLE_ADC1_CHAN1 ADC_CHANNEL_5

#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_11

static int adc_raw[2][10]; // Array to store raw ADC data


void app_main(void) {
    // ------------ ADC1 Init ------------
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    // ------------ ADC1 Config ------------
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = EXAMPLE_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN1, &config));

    while (1) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN0, &adc_raw[0][0]));
        ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1, EXAMPLE_ADC1_CHAN0, adc_raw[0][0]);
        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN1, &adc_raw[0][1]));
        ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1, EXAMPLE_ADC1_CHAN1, adc_raw[0][1]);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Tear Down
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
}
