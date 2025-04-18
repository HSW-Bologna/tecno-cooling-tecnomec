#include <esp_log.h>
#include "controller.h"
#include "model/model.h"
#include "adapters/view/view.h"
#include "gui.h"
#include "bsp/buttons.h"
#include "services/system_time.h"
#include "bsp/i2c_devices.h"
#include "bsp/relays.h"
#include "bsp/temperature.h"


static const char *TAG = __FILE_NAME__;


void controller_init(mut_model_t *model) {
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
    (void)model;

    {
        keypad_event_t event = bsp_button_manage();
        switch (event.tag) {
            case KEYPAD_EVENT_TAG_NOTHING:
                break;

            default:
                break;
        }
    }

    double temperature = bsp_temperature_manage();
    //ESP_LOGI(TAG, "%.f", temperature);

    controller_gui_manage();
}
