#include <assert.h>
#include <stdlib.h>
#include "lvgl.h"
#include "model/model.h"
#include "../view.h"
#include "../theme/style.h"
#include "../intl/intl.h"
#include <esp_log.h>


static const char *TAG = "PageMain";


struct page_data {
    view_controller_msg_t cmsg;
};


static void *create_page(pman_handle_t handle, void *extra) {
    (void)handle;
    (void)extra;

    struct page_data *pdata = lv_malloc(sizeof(struct page_data));
    assert(pdata != NULL);
    return pdata;
}


static void open_page(pman_handle_t handle, void *state) {
    struct page_data *pdata = state;
    (void)pdata;

    model_t *p_model = pman_get_user_data(handle);

    lv_obj_t *btn, *lbl;

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_style_bg_color(btn, lv_color_make(0xFF, 0, 0), LV_STATE_DEFAULT);
    lbl = lv_label_create(btn);
    lv_label_set_text(lbl, view_intl_get_string(p_model, STRINGS_HELLO_WORLD));
    lv_obj_center(lbl);
    lv_obj_center(btn);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 64, 64);
    lv_obj_set_style_bg_color(btn, lv_color_make(0, 0xff, 0), LV_STATE_DEFAULT);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 16, -16);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_style_bg_color(btn, lv_color_make(0, 0, 0xff), LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -48);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 16, 16);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -16, 16);

    ESP_LOGI(TAG, "Main open");
}


static pman_msg_t page_event(pman_handle_t handle, void *state, pman_event_t event) {
    (void)handle;

    pman_msg_t msg = PMAN_MSG_NULL;

    struct page_data *pdata = state;

    msg.user_msg    = &pdata->cmsg;
    pdata->cmsg.tag = VIEW_CONTROLLER_MESSAGE_TAG_NOTHING;

    switch (event.tag) {
        default:
            break;
    }

    return msg;
}


const pman_page_t page_main = {
    .create        = create_page,
    .destroy       = pman_destroy_all,
    .open          = open_page,
    .close         = pman_close_all,
    .process_event = page_event,
};
