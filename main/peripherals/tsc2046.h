#ifndef __TSC2046_H__
#define __TSC2046_H__


#include <stdint.h>
#include <stdbool.h>
// TODO: togliere questa dipendenza
#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define XPT2046_IRQ 21

#define XPT2046_AVG     4
#define XPT2046_XY_SWAP 1
#define XPT2046_X_MIN   150
#define XPT2046_Y_MIN   150
#define XPT2046_X_MAX   3850
#define XPT2046_Y_MAX   4095
#define XPT2046_X_INV   0
#define XPT2046_Y_INV   1

#define TP_SPI_CS 15


/*
 *  Funzione che inserisce in `data` le coordinate del tocco. E' thread safe
 * con la funzione `xpt2046_read`
 */
void tsc2046_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

/*
 *  Inizializza il dispositivo spi
 */
void tsc2046_init();

#endif
