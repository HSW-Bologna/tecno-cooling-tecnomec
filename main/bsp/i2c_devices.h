#ifndef BSP_I2C_DEVICES_INCLUDED
#define BSP_I2C_DEVICES_INCLUDED


#include "i2c_devices/rtc/RX8010/rx8010.h"
#include "i2c_devices/io/MCP23008/mcp23008.h"


extern i2c_driver_t bsp_rtc_driver;
extern i2c_driver_t bsp_io_expander_driver;


void bsp_i2c_devices_init(void);


#endif
