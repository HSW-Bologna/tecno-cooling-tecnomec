#include <driver/i2c.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "lvgl_helpers.h"
#include "lvgl_i2c/i2c_manager.h"
#include "model/model.h"
#include "view/view.h"
#include "controller/controller.h"
#include "controller/gui.h"
#include "peripherals/tsc2046.h"

static const char *TAG = "Main";

void app_main(void) {
    mut_model_t     model   = {0};
    model_updater_t updater = model_updater_init(&model);

    //lvgl_i2c_init(I2C_NUM_0);
    lvgl_driver_init();
    tsc2046_init();

    model_init(&model);
    view_init(updater, controller_process_message, disp_driver_flush, tsc2046_touch_read);
    controller_init(updater);

    ESP_LOGI(TAG, "Begin main loop");
    for (;;) {
        controller_manage(updater);

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
