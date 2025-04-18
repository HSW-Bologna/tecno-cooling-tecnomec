#ifndef BSP_PWM_H_INCLUDED
#define BSP_PWM_H_INCLUDED


#include <stdint.h>


void bsp_pwm_init(void);
void bsp_pwm_set(uint8_t percentage);


#endif
