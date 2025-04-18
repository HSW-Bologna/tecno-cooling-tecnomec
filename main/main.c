#include <driver/i2c.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "model/model.h"
#include "adapters/view/view.h"
#include "controller/controller.h"
#include "controller/gui.h"
#include "bsp/system.h"
#include "bsp/buttons.h"
#include "bsp/inputs.h"
#include "bsp/leds.h"
#include "bsp/buzzer.h"
#include "bsp/temperature.h"
#include "bsp/i2c_devices.h"
#include "bsp/tft/display.h"


static const char *TAG = "Main";


void app_main(void) {
    ESP_LOGI(TAG, "Main");

    mut_model_t model = {0};

    bsp_system_init();
    bsp_buttons_init();
    bsp_buzzer_init();
    bsp_inputs_init();
    bsp_leds_init();
    bsp_temperature_init();
    bsp_tft_display_init(view_display_flush_ready, VIEW_LVGL_BUFFER_SIZE);
    bsp_i2c_devices_init();

    model_init(&model);
    view_init(&model, controller_process_message, bsp_tft_display_lvgl_flush_cb, NULL);
    controller_init(&model);

    bsp_buzzer_beep(1, 100, 50, 10);
    ESP_LOGI(TAG, "Begin main loop");

    for (;;) {
        controller_manage(&model);

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
