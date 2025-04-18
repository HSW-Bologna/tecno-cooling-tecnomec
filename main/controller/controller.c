#include <esp_log.h>
#include "controller.h"
#include "model/model.h"
#include "adapters/view/view.h"
#include "gui.h"
#include "services/system_time.h"
#include "bsp/buttons.h"
#include "bsp/i2c_devices.h"
#include "bsp/relays.h"
#include "bsp/temperature.h"
#include "bsp/adc.h"
#include "bsp/buzzer.h"
#include "bsp/inputs.h"
#include "observer.h"


static const char *TAG = __FILE_NAME__;


void controller_init(mut_model_t *model) {
    observer_init(model);

    if (rx8010_is_stopped(bsp_rtc_driver)) {
        ESP_LOGI(TAG, "RTC was stopped, initializing...");
        rtc_time_t rtc_time = {.day = 7, .wday = 3, .month = 11, .year = 24};
        rx8010_set_stop(bsp_rtc_driver, 0);
        rx8010_set_time(bsp_rtc_driver, rtc_time);
        ESP_LOGI(TAG, "RTC Clock started");
    } else {
        rtc_time_t rtc_time = {0};
        rx8010_get_time(bsp_rtc_driver, &rtc_time);
        struct tm tm = rx8010_tm_from_rtc(rtc_time);
        tm.tm_isdst  = -1;
        system_time_set(&tm);
    }

    view_change_page(&page_main);
}


void controller_process_message(pman_handle_t handle, void *msg) {
    (void)handle;
    (void)msg;
}


void controller_manage(mut_model_t *model) {
    if (bsp_inputs_manage()) {
        model->run.sensors.inputs = bsp_inputs_get_map();
    }

    if (bsp_temperature_manage()) {
        model->run.sensors.temperature = (int16_t)(bsp_temperature_get() * 10);
    }

    if (bsp_adc_manage()) {
        model->run.sensors.humidity = bsp_adc_get(BSP_ADC_HUMIDITY);
        model->run.sensors.pressure = bsp_adc_get(BSP_ADC_PRESSURE);
    }

    {
        keypad_event_t event = bsp_button_manage();
        switch (event.tag) {
            case KEYPAD_EVENT_TAG_NOTHING:
                break;

            case KEYPAD_EVENT_TAG_CLICK: {
                ESP_LOGI(TAG, "Click %i", event.code);
                bsp_buzzer_beep(1, 20, 0, 10);
                view_event((view_event_t){.tag = VIEW_EVENT_TAG_BUTTON_CLICK, .as.button_click = event.code});
                break;
            }

            default:
                break;
        }
    }

    observer_manage(model);
    controller_gui_manage();
}
