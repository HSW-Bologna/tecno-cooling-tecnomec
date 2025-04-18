#ifndef BSP_BUTTONS_H_INCLUDED
#define BSP_BUTTONS_H_INCLUDED


#include "keypad.h"


typedef enum {
    BSP_BUTTON_1 = 0,
    BSP_BUTTON_2,
    BSP_BUTTON_3,
    BSP_BUTTON_4,
    BSP_BUTTON_5,
    BSP_BUTTON_6,
    BSP_BUTTON_7,
    BSP_BUTTON_8,
} bsp_button_t;


void           bsp_buttons_init(void);
keypad_event_t bsp_button_manage(void);


#endif
