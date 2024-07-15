#include <esp_log.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_ili9488.h>
#include <esp_lcd_touch_gt911.h>
#include <driver/i2c.h>
#include "touch.h"
#include "bsp/hardwareprofile.h"


static const char            *TAG = "Touch";
static esp_lcd_touch_handle_t tp;


void bsp_tft_touch_init(void) {
    gpio_config_t config = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask = BIT64(BSP_HAP_RESET_T),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&config));

    esp_lcd_panel_io_i2c_config_t io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();

    esp_lcd_touch_config_t tp_cfg = {
        .x_max        = 480,
        .y_max        = 320,
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
        .levels =
            {
                .reset     = 0,
                .interrupt = 0,
            },
        .flags =
            {
                .swap_xy  = 0,
                .mirror_x = 0,
                .mirror_y = 0,
            },
    };

    esp_lcd_panel_io_handle_t io_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)I2C_NUM_0, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install panel IO");
    size_t counter = 0;
    do {
        gpio_set_level(BSP_HAP_RESET_T, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(BSP_HAP_RESET_T, 1);
        vTaskDelay(pdMS_TO_TICKS(100));

        esp_err_t res = esp_lcd_touch_new_i2c_gt911(io_handle, &tp_cfg, &tp);
        if (res != ESP_OK) {
            ESP_LOGW(TAG, "Error in inizializing touch, retrying...");
            vTaskDelay(pdMS_TO_TICKS(200));
        } else {
            break;
        }
    } while (counter++ < 5);
    if (counter >= 5) {
        ESP_LOGE(TAG, "Unable to inizialize touch!");
    }
}


void bsp_tft_touch_read(lv_indev_t *indev, lv_indev_data_t *data) {
    static int16_t last_x = 0;
    static int16_t last_y = 0;

    esp_lcd_touch_read_data(tp);
    uint16_t touch_x[1];
    uint16_t touch_y[1];
    uint16_t touch_strength[1];
    uint8_t  touch_cnt = 0;

    bool touchpad_pressed = esp_lcd_touch_get_coordinates(tp, touch_x, touch_y, touch_strength, &touch_cnt, 1);

    if (touchpad_pressed) {
        last_x = BUILD_CONFIG_DISPLAY_HORIZONTAL_RESOLUTION - touch_y[0];
        last_y = touch_x[0];

        data->point.x = last_x;
        data->point.y = last_y;

        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
