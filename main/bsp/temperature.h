#ifndef BSP_TEMPERATURE_H_INCLUDED
#define BSP_TEMPERATURE_H_INCLUDED


#include <stdint.h>


void    bsp_temperature_init(void);
uint8_t bsp_temperature_manage(void);
double  bsp_temperature_get(void);


#endif
