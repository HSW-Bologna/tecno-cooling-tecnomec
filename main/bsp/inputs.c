#include "hardwareprofile.h"
#include "inputs.h"
#include "debounce.h"
#include "services/timestamp.h"
#include "i2c_devices.h"


static debounce_filter_t filter = {0};


void bsp_inputs_init(void) {
    gpio_config_t config = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode      = GPIO_MODE_INPUT,
        .pin_bit_mask =
            BIT64(BSP_HAP_IN1) | BIT64(BSP_HAP_IN2) | BIT64(BSP_HAP_IN3) | BIT64(BSP_HAP_IN4) | BIT64(BSP_HAP_IN5),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&config));
}


uint8_t bsp_inputs_manage(void) {
    static timestamp_t ts = 0;

    int level = 0;
    mcp23008_get_gpio_level(bsp_io_expander_driver, MCP23008_GPIO_8, &level);

    if (timestamp_is_expired(ts, timestamp_get())) {
        unsigned int bits = ((gpio_get_level(BSP_HAP_IN1) == 0) << 0) | ((gpio_get_level(BSP_HAP_IN2) == 0) << 1) |
                            ((gpio_get_level(BSP_HAP_IN3) == 0) << 2) | ((gpio_get_level(BSP_HAP_IN4) == 0) << 3) |
                            ((gpio_get_level(BSP_HAP_IN5) == 0) << 4) | ((level == 0) << 5);

        return debounce_filter(&filter, bits, 10);
    } else {
        return 0;
    }
}


uint8_t bsp_inputs_get_map(void) {
    return debounce_value(&filter);
}
