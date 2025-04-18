#include <driver/gpio.h>
#include <driver/ledc.h>
#include <driver/spi_master.h>
#include <esp_err.h>
#include <esp_freertos_hooks.h>
#include <esp_log.h>
#include <esp_lcd_panel_interface.h>
#include <esp_lcd_panel_commands.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_ili9488.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lvgl.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "../hardwareprofile.h"
#include "display.h"


static const char *TAG = "TftDisplay";

static const int          DISPLAY_COMMAND_BITS   = 8;
static const int          DISPLAY_PARAMETER_BITS = 8;
static const unsigned int DISPLAY_REFRESH_HZ     = 40000000;
static const int          DISPLAY_SPI_QUEUE_LEN  = 8;

// Default to 25 lines of color data

static const ledc_mode_t      BACKLIGHT_LEDC_MODE             = LEDC_LOW_SPEED_MODE;
static const ledc_channel_t   BACKLIGHT_LEDC_CHANNEL          = LEDC_CHANNEL_0;
static const ledc_timer_t     BACKLIGHT_LEDC_TIMER            = LEDC_TIMER_1;
static const ledc_timer_bit_t BACKLIGHT_LEDC_TIMER_RESOLUTION = LEDC_TIMER_10_BIT;
static const uint32_t         BACKLIGHT_LEDC_FRQUENCY         = 5000;

static esp_lcd_panel_io_handle_t lcd_io_handle = NULL;
static esp_lcd_panel_handle_t    lcd_handle    = NULL;

static void (*display_flush_ready)(void) = NULL;
static esp_err_t panel_st7789_init(esp_lcd_panel_t *panel);


static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata,
                                    void *user_ctx) {
    (void)panel_io;
    (void)edata;
    (void)user_ctx;
    if (display_flush_ready) {
        display_flush_ready();
    }
    return false;
}


void bsp_tft_display_lvgl_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map) {
    (void)display;

    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    esp_lcd_panel_draw_bitmap(lcd_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, px_map);
}


void bsp_tft_display_init(void (*display_flush_ready_cb)(void), size_t buffer_size) {
    display_flush_ready = display_flush_ready_cb;

    const ledc_channel_config_t LCD_backlight_channel = {
        .gpio_num   = BSP_HAP_RETRO,
        .speed_mode = BACKLIGHT_LEDC_MODE,
        .channel    = BACKLIGHT_LEDC_CHANNEL,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = BACKLIGHT_LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0,
        .flags      = {.output_invert = 0},
    };
    const ledc_timer_config_t LCD_backlight_timer = {
        .speed_mode      = BACKLIGHT_LEDC_MODE,
        .duty_resolution = BACKLIGHT_LEDC_TIMER_RESOLUTION,
        .timer_num       = BACKLIGHT_LEDC_TIMER,
        .freq_hz         = BACKLIGHT_LEDC_FRQUENCY,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_LOGI(TAG, "Initializing LEDC for backlight pin: %d", BSP_HAP_RETRO);

    ESP_ERROR_CHECK(ledc_timer_config(&LCD_backlight_timer));
    ESP_ERROR_CHECK(ledc_channel_config(&LCD_backlight_channel));

    const esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num         = BSP_HAP_CS_D,
        .dc_gpio_num         = BSP_HAP_D,
        .spi_mode            = 0,
        .pclk_hz             = DISPLAY_REFRESH_HZ,
        .trans_queue_depth   = DISPLAY_SPI_QUEUE_LEN,
        .on_color_trans_done = notify_lvgl_flush_ready,
        .user_ctx            = NULL,
        .lcd_cmd_bits        = DISPLAY_COMMAND_BITS,
        .lcd_param_bits      = DISPLAY_PARAMETER_BITS,
        .flags               = {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
            .dc_as_cmd_phase = 0, .dc_low_on_data = 0, .octal_mode = 0, .lsb_first = 0
#else
            .dc_low_on_data = 0, .octal_mode = 0, .sio_mode = 0, .lsb_first = 0, .cs_high_active = 0
#endif
        }};

    const esp_lcd_panel_dev_config_t lcd_config = {
        .reset_gpio_num = BSP_HAP_RESET_D,
        .data_endian    = LCD_RGB_DATA_ENDIAN_LITTLE,
        .rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .flags          = {.reset_active_high = 0},
        .vendor_config  = NULL,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &lcd_io_handle));

    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(lcd_io_handle, &lcd_config, &lcd_handle));
    lcd_handle->init = panel_st7789_init;

    ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(lcd_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(lcd_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(lcd_handle, false, true));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(lcd_handle, 0, 0));
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    ESP_ERROR_CHECK(esp_lcd_panel_disp_off(lcd_handle, false));
#else
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_handle, true));
#endif

    bsp_tft_display_brightness_set(100);
}


void bsp_tft_display_brightness_set(uint8_t brightness_percentage) {
    if (brightness_percentage > 100) {
        brightness_percentage = 100;
    }

    // LEDC resolution set to 10bits, thus: 100% = 1023
    uint32_t duty_cycle = (1023 * brightness_percentage) / 100;
    ESP_ERROR_CHECK(ledc_set_duty(BACKLIGHT_LEDC_MODE, BACKLIGHT_LEDC_CHANNEL, duty_cycle));
    ESP_ERROR_CHECK(ledc_update_duty(BACKLIGHT_LEDC_MODE, BACKLIGHT_LEDC_CHANNEL));
}


typedef struct {
    esp_lcd_panel_t           base;
    esp_lcd_panel_io_handle_t io;
    int                       reset_gpio_num;
    bool                      reset_level;
    int                       x_gap;
    int                       y_gap;
    uint8_t                   fb_bits_per_pixel;
    uint8_t                   madctl_val;     // save current value of LCD_CMD_MADCTL register
    uint8_t                   colmod_val;     // save current value of LCD_CMD_COLMOD register
    uint8_t                   ramctl_val_1;
    uint8_t                   ramctl_val_2;
} st7789_panel_t;

#define ST7789_CMD_RAMCTRL 0xb0

static esp_err_t panel_st7789_init(esp_lcd_panel_t *panel) {
    st7789_panel_t *st7789 = __containerof(panel, st7789_panel_t, base);
    // LCD goes into sleep mode and display will be turned off after power on reset, exit sleep mode first
    esp_lcd_panel_io_tx_param(lcd_io_handle, LCD_CMD_SLPOUT, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_lcd_panel_io_tx_param(lcd_io_handle, LCD_CMD_MADCTL, (uint8_t[]){st7789->madctl_val}, 1);
    esp_lcd_panel_io_tx_param(lcd_io_handle, LCD_CMD_COLMOD,
                              (uint8_t[]){// st7789->colmod_val,
                                          0x5},
                              1);
    esp_lcd_panel_io_tx_param(lcd_io_handle, ST7789_CMD_RAMCTRL,
                              (uint8_t[]){st7789->ramctl_val_1, st7789->ramctl_val_2}, 2);

    return ESP_OK;
}
