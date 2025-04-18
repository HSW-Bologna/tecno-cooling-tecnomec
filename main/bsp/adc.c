#include "adc.h"
#include "hardwareprofile.h"
#include "adc.h"
#include <driver/adc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "services/timestamp.h"
#include "i2c_devices.h"


#define ATTENUATION    ADC_ATTEN_DB_12
#define AVERAGE_LENGTH 10


static bool     example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten,
                                             adc_cali_handle_t *out_handle);
static uint16_t read_adc_channel_mv(bsp_adc_t adc);


const static char         *TAG                        = __FILE_NAME__;
static const adc_channel_t adc_channels[BSP_ADCS_NUM] = {BSP_HAP_ADC_CHANNEL_UMIDITA__M, BSP_HAP_ADC_CHANNEL_PRESS};
static struct {
    adc_oneshot_unit_handle_t adc_handle;
    adc_cali_handle_t         calibration_handles[BSP_ADCS_NUM];
    uint8_t                   readings_full_circle;
    size_t                    readings_index;
    uint16_t                  readings[BSP_ADCS_NUM][AVERAGE_LENGTH];
} state = {0};


void bsp_adc_init(void) {
    //-------------ADC1 Init---------------//
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &state.adc_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten    = ATTENUATION,
    };
    for (size_t i = 0; i < BSP_ADCS_NUM; i++) {
        ESP_ERROR_CHECK(adc_oneshot_config_channel(state.adc_handle, adc_channels[i], &config));
        //-------------ADC1 Calibration Init---------------//
        example_adc_calibration_init(ADC_UNIT_1, adc_channels[i], ATTENUATION, &state.calibration_handles[i]);
    }

    mcp23008_set_gpio_level(bsp_io_expander_driver, MCP23008_GPIO_7, 1);
}


uint8_t bsp_adc_manage(void) {
    static timestamp_t ts = 0;

    if (timestamp_is_expired(ts, 100)) {
        for (size_t i = 0; i < BSP_ADCS_NUM; i++) {
            if (i == BSP_ADC_HUMIDITY) {
                mcp23008_set_gpio_level(bsp_io_expander_driver, MCP23008_GPIO_7, 0);
                vTaskDelay(1);
            }
            state.readings[i][state.readings_index] = read_adc_channel_mv(i);
            if (i == BSP_ADC_HUMIDITY) {
                mcp23008_set_gpio_level(bsp_io_expander_driver, MCP23008_GPIO_7, 1);
            }

            if (state.readings_index == AVERAGE_LENGTH - 1) {
                state.readings_full_circle = 1;
            }

            state.readings_index = (state.readings_index + 1) % AVERAGE_LENGTH;
        }

        ts = timestamp_get();

        return 1;
    } else {
        return 0;
    }
}


uint16_t bsp_adc_get(bsp_adc_t adc) {
    uint32_t readings = state.readings_full_circle ? AVERAGE_LENGTH : state.readings_index;
    uint32_t total    = 0;

    if (readings > 0) {
        for (size_t i = 0; i < readings; i++) {
            total += state.readings[adc][i];
        }

        return (uint16_t)(total / readings);
    } else {
        return 0;
    }
}


/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten,
                                         adc_cali_handle_t *out_handle) {
    adc_cali_handle_t handle     = NULL;
    esp_err_t         ret        = ESP_FAIL;
    bool              calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id  = unit,
            .chan     = channel,
            .atten    = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id  = unit,
            .atten    = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

static uint16_t read_adc_channel_mv(bsp_adc_t adc) {
    int raw     = 0;
    int voltage = 0;

    ESP_ERROR_CHECK(adc_oneshot_read(state.adc_handle, adc_channels[adc], &raw));
    //ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, adc_channels[adc], raw);
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(state.calibration_handles[adc], raw, &voltage));
    //ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, adc_channels[adc], voltage);

    return (uint16_t)voltage;
}
