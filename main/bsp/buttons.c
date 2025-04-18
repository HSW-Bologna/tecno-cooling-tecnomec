#include "hardwareprofile.h"
#include "buttons.h"
#include "keypad.h"
#include "services/timestamp.h"
#include <esp_log.h>


static const char  *TAG    = __FILE_NAME__;
static keypad_key_t keys[] = {
    KEYPAD_KEY(0x01, BSP_BUTTON_1), KEYPAD_KEY(0x02, BSP_BUTTON_2), KEYPAD_KEY(0x04, BSP_BUTTON_3),
    KEYPAD_KEY(0x08, BSP_BUTTON_4), KEYPAD_KEY(0x10, BSP_BUTTON_5), KEYPAD_KEY(0x20, BSP_BUTTON_6),
    KEYPAD_KEY(0x40, BSP_BUTTON_7), KEYPAD_KEY(0x80, BSP_BUTTON_8), KEYPAD_NULL_KEY,
};


void bsp_buttons_init(void) {
    gpio_config_t config = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_INPUT,
        .pin_bit_mask = BIT64(BSP_HAP_P1) | BIT64(BSP_HAP_P2) | BIT64(BSP_HAP_P3) | BIT64(BSP_HAP_P4) |
                        BIT64(BSP_HAP_P5) | BIT64(BSP_HAP_P6) | BIT64(BSP_HAP_P7) | BIT64(BSP_HAP_P8),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&config));
}


keypad_event_t bsp_button_manage(void) {
    unsigned long bits = ((gpio_get_level(BSP_HAP_P1) == 0) << 0) | ((gpio_get_level(BSP_HAP_P2) == 0) << 1) |
                         ((gpio_get_level(BSP_HAP_P3) == 0) << 2) | ((gpio_get_level(BSP_HAP_P4) == 0) << 3) |
                         ((gpio_get_level(BSP_HAP_P5) == 0) << 4) | ((gpio_get_level(BSP_HAP_P6) == 0) << 5) |
                         ((gpio_get_level(BSP_HAP_P7) == 0) << 6) | ((gpio_get_level(BSP_HAP_P8) == 0) << 7);

    return keypad_routine(keys, 50, 3000, 200, timestamp_get(), bits);
}
