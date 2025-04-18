#ifndef BSP_ADC_H_INCLUDED
#define BSP_ADC_H_INCLUDED


#include <stdint.h>


typedef enum {
    BSP_ADC_HUMIDITY = 0,
    BSP_ADC_PRESSURE,
#define BSP_ADCS_NUM 2
} bsp_adc_t;


void     bsp_adc_init(void);
uint8_t  bsp_adc_manage(void);
uint16_t bsp_adc_get(bsp_adc_t adc);


#endif
