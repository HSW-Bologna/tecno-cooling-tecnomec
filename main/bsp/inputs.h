#ifndef BSP_INPUTS_H_INCLUDED
#define BSP_INPUTS_H_INCLUDED


#include <stdint.h>


void    bsp_inputs_init(void);
uint8_t bsp_inputs_manage(void);
uint8_t bsp_inputs_get_map(void);


#endif
