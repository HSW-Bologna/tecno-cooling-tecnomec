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


struct page_data {
    lv_obj_t *leds[INPUTS_NUM];
    lv_obj_t *label_temperature;
    lv_obj_t *label_humidity;
    lv_obj_t *label_pressure;
};


static void update_page(model_t *model, struct page_data *pdata);


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

    view_common_title_create(lv_screen_active(), "Diagnosi ingressi");

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

    for (size_t i = 0; i < INPUTS_NUM; i++) {
        lv_obj_t *cont_input = lv_obj_create(cont);
        lv_obj_remove_flag(cont_input, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(cont_input, 48, 48);

        lv_obj_t *led = lv_led_create(cont_input);
        lv_obj_set_size(led, 36, 36);
        lv_led_set_color(led, lv_color_make(0, 0xFF, 0));
        lv_obj_center(led);
        pdata->leds[i] = led;

        lv_obj_t *label = lv_label_create(cont_input);
        lv_obj_set_style_text_font(label, STYLE_FONT_SMALL, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(label, "IN %zu", i + 1);
        lv_obj_center(label);
    }

    {
        lv_obj_t *cont_input = lv_obj_create(cont);
        lv_obj_remove_flag(cont_input, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(cont_input, 200, 48);

        lv_obj_t *label = lv_label_create(cont_input);
        lv_obj_set_style_text_font(label, STYLE_FONT_SMALL, LV_STATE_DEFAULT);
        lv_obj_center(label);
        pdata->label_temperature = label;
    }

    {
        lv_obj_t *cont_input = lv_obj_create(cont);
        lv_obj_remove_flag(cont_input, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(cont_input, 150, 48);

        lv_obj_t *label = lv_label_create(cont_input);
        lv_obj_set_style_text_font(label, STYLE_FONT_SMALL, LV_STATE_DEFAULT);
        lv_obj_center(label);
        pdata->label_humidity = label;
    }

    {
        lv_obj_t *cont_input = lv_obj_create(cont);
        lv_obj_remove_flag(cont_input, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(cont_input, 150, 48);

        lv_obj_t *label = lv_label_create(cont_input);
        lv_obj_set_style_text_font(label, STYLE_FONT_SMALL, LV_STATE_DEFAULT);
        lv_obj_center(label);
        pdata->label_pressure = label;
    }

    model_t *model = pman_get_user_data(handle);
    VIEW_ADD_WATCHED_VARIABLE(&model->run.sensors.inputs, 0);
    VIEW_ADD_WATCHED_VARIABLE(&model->run.sensors.temperature, 0);
    VIEW_ADD_WATCHED_VARIABLE(&model->run.sensors.humidity, 0);
    VIEW_ADD_WATCHED_VARIABLE(&model->run.sensors.pressure, 0);

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
                            msg.stack_msg = PMAN_STACK_MSG_SWAP(&page_test_relays);
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
    for (size_t i = 0; i < INPUTS_NUM; i++) {
        if ((model->run.sensors.inputs & (1 << i)) > 0) {
            lv_led_set_brightness(pdata->leds[i], LV_LED_BRIGHT_MAX);
        } else {
            lv_led_set_brightness(pdata->leds[i], LV_LED_BRIGHT_MIN);
        }
    }

    lv_label_set_text_fmt(pdata->label_temperature, "Temperatura: %.1fC", (float)model->run.sensors.temperature / 10.);
    lv_label_set_text_fmt(pdata->label_humidity, "Umidita': %i%%", model->run.sensors.humidity);
    lv_label_set_text_fmt(pdata->label_pressure, "Pressione: %i", model->run.sensors.pressure);
}


const pman_page_t page_test_inputs = {
    .create        = create_page,
    .destroy       = pman_destroy_all,
    .open          = open_page,
    .close         = pman_close_all,
    .process_event = page_event,
};
