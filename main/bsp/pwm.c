#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_err.h>
#include <esp_log.h>
#include <lvgl.h>
#include <stdio.h>
#include "pwm.h"
#include "hardwareprofile.h"


static const char            *TAG                       = __FILE_NAME__;
static const ledc_mode_t      PWM_LEDC_MODE             = LEDC_LOW_SPEED_MODE;
static const ledc_channel_t   PWM_LEDC_CHANNEL          = LEDC_CHANNEL_1;
static const ledc_timer_t     PWM_LEDC_TIMER            = LEDC_TIMER_2;
static const ledc_timer_bit_t PWM_LEDC_TIMER_RESOLUTION = LEDC_TIMER_10_BIT;
static const uint32_t         PWM_LEDC_FRQUENCY         = 5000;


void bsp_pwm_init(void) {
    const ledc_channel_config_t LCD_backlight_channel = {
        .gpio_num   = BSP_HAP_0_10_M,
        .speed_mode = PWM_LEDC_MODE,
        .channel    = PWM_LEDC_CHANNEL,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = PWM_LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0,
        .flags      = {.output_invert = 0},
    };
    const ledc_timer_config_t LCD_backlight_timer = {
        .speed_mode      = PWM_LEDC_MODE,
        .duty_resolution = PWM_LEDC_TIMER_RESOLUTION,
        .timer_num       = PWM_LEDC_TIMER,
        .freq_hz         = PWM_LEDC_FRQUENCY,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_LOGI(TAG, "Initializing LEDC for backlight pin: %d", BSP_HAP_RETRO);

    ESP_ERROR_CHECK(ledc_timer_config(&LCD_backlight_timer));
    ESP_ERROR_CHECK(ledc_channel_config(&LCD_backlight_channel));

    bsp_pwm_set(0);
}


void bsp_pwm_set(uint8_t percentage) {
    if (percentage > 100) {
        percentage = 100;
    }

    // LEDC resolution set to 10bits, thus: 100% = 1023
    uint32_t duty_cycle = (1023 * percentage) / 100;
    ESP_ERROR_CHECK(ledc_set_duty(PWM_LEDC_MODE, PWM_LEDC_CHANNEL, duty_cycle));
    ESP_ERROR_CHECK(ledc_update_duty(PWM_LEDC_MODE, PWM_LEDC_CHANNEL));
}
