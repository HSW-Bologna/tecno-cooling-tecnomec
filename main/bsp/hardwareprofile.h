#ifndef HARDWAREPROFILE_H_INCLUDED
#define HARDWAREPROFILE_H_INCLUDED


#include <driver/gpio.h>


/*
 * Definizioni dei pin da utilizzare
 */

#define BSP_HAP_SDA GPIO_NUM_6
#define BSP_HAP_SCL GPIO_NUM_7

#define BSP_HAP_CS_D    GPIO_NUM_17
#define BSP_HAP_D       GPIO_NUM_15
#define BSP_HAP_CLK_D   GPIO_NUM_16
#define BSP_HAP_DOUT_D  GPIO_NUM_14
#define BSP_HAP_MISO_D  GPIO_NUM_13
#define BSP_HAP_RESET_D GPIO_NUM_12
#define BSP_HAP_RETRO   GPIO_NUM_18

#define BSP_HAP_0_10_M GPIO_NUM_2

#define BSP_HAP_BUZ     GPIO_NUM_46
#define BSP_HAP_SDA_CLK GPIO_NUM_40
#define BSP_HAP_SCL_CLK GPIO_NUM_41

#define BSP_HAP_P1 GPIO_NUM_4
#define BSP_HAP_P2 GPIO_NUM_20
#define BSP_HAP_P3 GPIO_NUM_34
#define BSP_HAP_P4 GPIO_NUM_5
#define BSP_HAP_P5 GPIO_NUM_47
#define BSP_HAP_P6 GPIO_NUM_26
#define BSP_HAP_P7 GPIO_NUM_19
#define BSP_HAP_P8 GPIO_NUM_21

#define BSP_HAP_LED1 GPIO_NUM_10
#define BSP_HAP_LED2 GPIO_NUM_11
#define BSP_HAP_LED3 GPIO_NUM_8
#define BSP_HAP_LED4 GPIO_NUM_9

#define BSP_HAP_IN1 GPIO_NUM_48
#define BSP_HAP_IN2 GPIO_NUM_35
#define BSP_HAP_IN3 GPIO_NUM_36
#define BSP_HAP_IN4 GPIO_NUM_37
#define BSP_HAP_IN5 GPIO_NUM_38

#define BSP_HAP_CS_PT100 GPIO_NUM_39

#define BSP_HAP_ADC_CHANNEL_UMIDITA__M ADC_CHANNEL_0
#define BSP_HAP_ADC_CHANNEL_PRESS      ADC_CHANNEL_2

#endif
