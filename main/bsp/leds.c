#include "hardwareprofile.h"
#include "leds.h"


void bsp_leds_init(void) {
    gpio_config_t config = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask = BIT64(BSP_HAP_LED1) | BIT64(BSP_HAP_LED2) | BIT64(BSP_HAP_LED3) | BIT64(BSP_HAP_LED4),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&config));
}
