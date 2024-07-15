#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED


#include "model/model.h"
#include "page_manager.h"


#define VIEW_LVGL_BUFFER_SIZE (BUILD_CONFIG_DISPLAY_HORIZONTAL_RESOLUTION * 10)


typedef enum {
    VIEW_CONTROLLER_MESSAGE_TAG_NOTHING = 0,
} view_controller_msg_tag_t;

typedef struct {
    view_controller_msg_tag_t tag;
    union {
    } as;
} view_controller_msg_t;


void view_init(model_t *p_model, pman_user_msg_cb_t controller_cb, lv_display_flush_cb_t flush_cb,
               lv_indev_read_cb_t read_cb);

void view_change_page(const pman_page_t *page);
void view_display_flush_ready(void);


extern const pman_page_t page_main;


#endif
