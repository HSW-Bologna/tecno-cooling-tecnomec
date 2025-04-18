#include "i2c_devices.h"
#include "relays.h"


void bsp_relays_set(bsp_relays_t relay, uint8_t value) {
    mcp23008_gpio_t transform[] = {
        MCP23008_GPIO_6, MCP23008_GPIO_5, MCP23008_GPIO_4, MCP23008_GPIO_3, MCP23008_GPIO_2, MCP23008_GPIO_1,
    };

    mcp23008_set_gpio_level(bsp_io_expander_driver, transform[relay], value);
}
