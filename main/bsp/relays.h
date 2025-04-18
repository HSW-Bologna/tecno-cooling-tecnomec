#ifndef BSP_RELAYS_H_INCLUDED
#define BSP_RELAYS_H_INCLUDED


#include <stdint.h>


typedef enum {
    BSP_RELAYS_1 = 0,
    BSP_RELAYS_2,
    BSP_RELAYS_3,
    BSP_RELAYS_4,
    BSP_RELAYS_5,
    BSP_RELAYS_6,
} bsp_relays_t;


void bsp_relays_set(bsp_relays_t relay, uint8_t value);


#endif
