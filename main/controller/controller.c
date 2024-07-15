#include "controller.h"
#include "model/model.h"
#include "adapters/view/view.h"
#include "gui.h"


void controller_init(mut_model_t *p_model) {
    (void)p_model;

    view_change_page(&page_main);
}


void controller_process_message(pman_handle_t handle, void *msg) {
    (void)handle;
    (void)msg;
}


void controller_manage(mut_model_t *p_model) {
    (void)p_model;

    controller_gui_manage();
}
