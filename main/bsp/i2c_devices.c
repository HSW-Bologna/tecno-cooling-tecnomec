#include "i2c_devices/rtc/RX8010/rx8010.h"
#include "i2c_devices/io/MCP23008/mcp23008.h"
#include "i2c_ports/esp-idf/esp_idf_i2c_port.h"
#include "i2c_devices.h"


i2c_driver_t bsp_rtc_driver = {
    .device_address = RX8010_DEFAULT_ADDRESS,
    .i2c_transfer   = esp_idf_i2c_port_transfer,
    .arg            = (void *)(uintptr_t)I2C_NUM_0,
};


i2c_driver_t bsp_io_expander_driver = {
    .device_address = MCP23008_DEFAULT_ADDR,
    .i2c_transfer   = esp_idf_i2c_port_transfer,
    .arg            = (void *)(uintptr_t)I2C_NUM_0,
};


void bsp_i2c_devices_init(void) {
    mcp23008_set_gpio_direction(bsp_io_expander_driver, MCP23008_GPIO_1, MCP23008_OUTPUT_MODE);
    mcp23008_set_gpio_direction(bsp_io_expander_driver, MCP23008_GPIO_2, MCP23008_OUTPUT_MODE);
    mcp23008_set_gpio_direction(bsp_io_expander_driver, MCP23008_GPIO_3, MCP23008_OUTPUT_MODE);
    mcp23008_set_gpio_direction(bsp_io_expander_driver, MCP23008_GPIO_4, MCP23008_OUTPUT_MODE);
    mcp23008_set_gpio_direction(bsp_io_expander_driver, MCP23008_GPIO_5, MCP23008_OUTPUT_MODE);
    mcp23008_set_gpio_direction(bsp_io_expander_driver, MCP23008_GPIO_6, MCP23008_OUTPUT_MODE);
    mcp23008_set_gpio_direction(bsp_io_expander_driver, MCP23008_GPIO_7, MCP23008_OUTPUT_MODE);
    mcp23008_set_gpio_direction(bsp_io_expander_driver, MCP23008_GPIO_8, MCP23008_INPUT_MODE);
}
