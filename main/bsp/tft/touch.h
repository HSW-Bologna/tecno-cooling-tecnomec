#ifndef TOUCH_H_INCLUDED
#define TOUCH_H_INCLUDED


#include <lvgl.h>


void bsp_tft_touch_init(void);
void bsp_tft_touch_read(lv_indev_t *indev, lv_indev_data_t *data);


#endif
