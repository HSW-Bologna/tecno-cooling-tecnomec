#ifndef BUZZER_H_INCLUDED
#define BUZZER_H_INCLUDED


#include <stdint.h>


void bsp_buzzer_init(void);
void bsp_buzzer_beep(uint16_t r, unsigned long t_on, unsigned long t_off, uint16_t tone);


#endif
