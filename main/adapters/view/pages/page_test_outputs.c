#include <assert.h>
#include <stdlib.h>
#include "lvgl.h"
#include "model/model.h"
#include "../view.h"
#include "../theme/style.h"
#include "../intl/intl.h"
#include <esp_log.h>
#include "../common.h"
#include "../style.h"


#define OUTPUTS_NUM (RELAYS_NUM + 1)


struct page_data {
    lv_obj_t *objs[OUTPUTS_NUM];
    lv_obj_t *leds[OUTPUTS_NUM];
    lv_obj_t *label_pwm;

    uint16_t selected;
};


static void update_page(model_t *model, struct page_data *pdata);


static void *create_page(pman_handle_t handle, void *extra) {
    (void)handle;
    (void)extra;

    struct page_data *pdata = lv_malloc(sizeof(struct page_data));
    assert(pdata != NULL);

    pdata->selected = 0;

    return pdata;
}


static void open_page(pman_handle_t handle, void *state) {
    struct page_data *pdata = state;
    (void)pdata;

    view_common_title_create(lv_screen_active(), "Diagnosi uscite");

    lv_obj_t *cont = lv_obj_create(lv_screen_active());
    lv_obj_set_size(cont, LV_PCT(100), LV_VER_RES - 50);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_pad_hor(cont, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_ver(cont, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cont, 8, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cont, 8, LV_STATE_DEFAULT);
    lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    for (size_t i = 0; i < RELAYS_NUM; i++) {
        lv_obj_t *cont_input = lv_obj_create(cont);
        lv_obj_remove_flag(cont_input, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(cont_input, 48, 48);
        lv_obj_set_style_border_width(cont_input, 4, LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(cont_input, lv_color_make(0xFF, 0xA0, 0xA0), LV_STATE_FOCUSED);
        pdata->objs[i] = cont_input;

        lv_obj_t *led = lv_led_create(cont_input);
        lv_obj_set_size(led, 36, 36);
        lv_led_set_color(led, lv_color_make(0xFF, 0, 0));
        lv_obj_center(led);
        pdata->leds[i] = led;

        lv_obj_t *label = lv_label_create(cont_input);
        lv_obj_set_style_text_font(label, STYLE_FONT_SMALL, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(label, "RL %zu", i + 1);
        lv_obj_center(label);
    }

    {
        lv_obj_t *cont_input = lv_obj_create(cont);
        lv_obj_remove_flag(cont_input, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(cont_input, 200, 48);
        lv_obj_set_style_border_width(cont_input, 4, LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(cont_input, lv_color_make(0xFF, 0xA0, 0xA0), LV_STATE_FOCUSED);
        pdata->objs[OUTPUTS_NUM - 1] = cont_input;

        lv_obj_t *label = lv_label_create(cont_input);
        lv_obj_set_style_text_font(label, STYLE_FONT_SMALL, LV_STATE_DEFAULT);
        lv_obj_center(label);
        pdata->label_pwm = label;
    }

    model_t *model = pman_get_user_data(handle);
    VIEW_ADD_WATCHED_VARIABLE(&model->run.tested_relay, 0);

    update_page(model, pdata);
}


static pman_msg_t page_event(pman_handle_t handle, void *state, pman_event_t event) {
    pman_msg_t msg = PMAN_MSG_NULL;

    mut_model_t      *model = pman_get_user_data(handle);
    struct page_data *pdata = state;

    switch (event.tag) {
        case PMAN_EVENT_TAG_OPEN: {
            model_enter_test_mode(model);
            break;
        }

        case PMAN_EVENT_TAG_USER: {
            view_event_t *view_event = event.as.user;
            switch (view_event->tag) {
                case VIEW_EVENT_TAG_BUTTON_CLICK: {
                    switch (view_event->as.button_click) {
                        case BSP_BUTTON_1:
                        case BSP_BUTTON_2: {
                            msg.stack_msg = PMAN_STACK_MSG_SWAP(&page_test_inputs);
                            break;
                        }

                        case BSP_BUTTON_4: {
                            if (pdata->selected == 0) {
                                pdata->selected = OUTPUTS_NUM - 1;
                            } else {
                                pdata->selected--;
                            }
                            update_page(model, pdata);
                            break;
                        }

                        case BSP_BUTTON_5: {
                            pdata->selected = (pdata->selected + 1) % OUTPUTS_NUM;
                            update_page(model, pdata);
                            break;
                        }

                        case BSP_BUTTON_3: {
                            if (pdata->selected < RELAYS_NUM) {
                                model_test_toggle_relay(model, pdata->selected);
                            } else if (model->run.test_pwm <= 95) {
                                model->run.test_pwm += 5;
                            } else {
                                model->run.test_pwm = 100;
                            }
                            update_page(model, pdata);
                            break;
                        }

                        case BSP_BUTTON_6: {
                            if (pdata->selected < RELAYS_NUM) {
                                model_test_toggle_relay(model, pdata->selected);
                            } else if (model->run.test_pwm >= 5) {
                                model->run.test_pwm -= 5;
                            } else {
                                model->run.test_pwm = 0;
                            }
                            update_page(model, pdata);
                            break;
                        }

                        case BSP_BUTTON_7: {
                            model_exit_test_mode(model);
                            msg.stack_msg = PMAN_STACK_MSG_BACK();
                            break;
                        }

                        default:
                            break;
                    }
                    break;
                }

                case VIEW_EVENT_TAG_PAGE_WATCHER: {
                    update_page(model, pdata);
                    break;
                }
            }

            break;
        }

        default:
            break;
    }

    return msg;
}


static void update_page(model_t *model, struct page_data *pdata) {
    for (size_t i = 0; i < RELAYS_NUM; i++) {
        if (pdata->selected == i) {
            lv_obj_add_state(pdata->objs[i], LV_STATE_FOCUSED);
        } else {
            lv_obj_remove_state(pdata->objs[i], LV_STATE_FOCUSED);
        }
        if (model->run.tested_relay == (int16_t)i) {
            lv_led_set_brightness(pdata->leds[i], LV_LED_BRIGHT_MAX);
        } else {
            lv_led_set_brightness(pdata->leds[i], LV_LED_BRIGHT_MIN);
        }
    }

    if (pdata->selected == OUTPUTS_NUM - 1) {
        lv_obj_add_state(pdata->objs[OUTPUTS_NUM - 1], LV_STATE_FOCUSED);
    } else {
        lv_obj_remove_state(pdata->objs[OUTPUTS_NUM - 1], LV_STATE_FOCUSED);
    }

    lv_label_set_text_fmt(pdata->label_pwm, "PWM: %3i%%", model->run.test_pwm);
}


const pman_page_t page_test_relays = {
    .create        = create_page,
    .destroy       = pman_destroy_all,
    .open          = open_page,
    .close         = pman_close_all,
    .process_event = page_event,
};
