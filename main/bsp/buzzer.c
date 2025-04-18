#include "hardwareprofile.h"
#include "buzzer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "driver/ledc.h"


#define BUZZER_SPEED_MODE   LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_TIMER   LEDC_TIMER_1
#define BUZZER_LEDC_CHANNEL LEDC_CHANNEL_1


static void buzzer_periodic(TimerHandle_t timer);
static void buzzer_on(uint16_t volume);
static void buzzer_off(void);


static const uint16_t volume[4]   = {0, 1, 15, 127};
static uint16_t       buzzer_tone = 2;
static unsigned long  time_on     = 0;
static unsigned long  time_off    = 0;
static size_t         repeat      = 0;
static int            is_set      = 0;
static int            is_on       = 0;
static TimerHandle_t  timer;
static const char    *TAG = "Buzzer";



void bsp_buzzer_init(void) {
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,      // resolution of PWM duty
        .freq_hz         = 3000,                  // frequency of PWM signal
        .speed_mode      = BUZZER_SPEED_MODE,     // timer mode
        .timer_num       = BUZZER_LEDC_TIMER,     // timer index
        .clk_cfg         = LEDC_AUTO_CLK,         // Auto select the source clock
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .channel    = BUZZER_LEDC_CHANNEL,
        .duty       = 0,
        .gpio_num   = BSP_HAP_BUZ,
        .speed_mode = BUZZER_SPEED_MODE,
        .hpoint     = 0,
        .timer_sel  = BUZZER_LEDC_TIMER,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    ledc_fade_func_install(0);

    static StaticTimer_t timer_buffer;
    timer = xTimerCreateStatic(TAG, pdMS_TO_TICKS(10), pdTRUE, NULL, buzzer_periodic, &timer_buffer);
}


void bsp_buzzer_beep(uint16_t r, unsigned long t_on, unsigned long t_off, uint16_t tone) {
    xTimerStop(timer, portMAX_DELAY);
    xTimerChangePeriod(timer, pdMS_TO_TICKS(t_on), portMAX_DELAY);

    buzzer_tone = tone;
    if (buzzer_tone > 3) {
        buzzer_tone = 3;
    }

    // No tone, no buzzer
    if (volume[buzzer_tone] > 0) {
        repeat   = r;
        time_on  = t_on;
        time_off = t_off;
        is_set   = 1;

        buzzer_on(volume[buzzer_tone]);
        xTimerStart(timer, portMAX_DELAY);
    }
}


static void buzzer_periodic(TimerHandle_t timer) {
    if (is_set && repeat > 0) {
        if (is_on) {
            buzzer_off();
            if (time_off > 0) {
                xTimerChangePeriod(timer, pdMS_TO_TICKS(time_off), portMAX_DELAY);
                is_on = 0;
            } else {
                xTimerChangePeriod(timer, pdMS_TO_TICKS(time_on), portMAX_DELAY);
                is_on = 1;
            }
            xTimerReset(timer, portMAX_DELAY);
            repeat--;
        } else {
            buzzer_on(volume[buzzer_tone]);
            is_on = 1;
            xTimerChangePeriod(timer, pdMS_TO_TICKS(time_on), portMAX_DELAY);
            xTimerReset(timer, portMAX_DELAY);
        }
    }

    if (is_set && repeat == 0) {
        is_set = 0;
        buzzer_off();
        xTimerStop(timer, portMAX_DELAY);
    }
}


static void buzzer_on(uint16_t volume) {
    ledc_set_duty(BUZZER_SPEED_MODE, BUZZER_LEDC_CHANNEL, volume);
    ledc_update_duty(BUZZER_SPEED_MODE, BUZZER_LEDC_CHANNEL);
}


static void buzzer_off(void) {
    ledc_set_duty(BUZZER_SPEED_MODE, BUZZER_LEDC_CHANNEL, 0);
    ledc_update_duty(BUZZER_SPEED_MODE, BUZZER_LEDC_CHANNEL);
}
