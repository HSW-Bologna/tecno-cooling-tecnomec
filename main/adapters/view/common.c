#include "view.h"
#include "style.h"


lv_obj_t *view_common_title_create(lv_obj_t *parent, const char *title) {
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_remove_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(cont, LV_PCT(100), 48);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);
    lv_label_set_text(label, title);
    lv_obj_center(label);

    return cont;
}
