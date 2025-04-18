#include "watcher.h"
#include "model/model.h"
#include "services/timestamp.h"
#include "bsp/relays.h"
#include "bsp/pwm.h"


static void relays_cb(void *old_value, const void *memory, uint16_t size, void *user_ptr, void *arg);
static void pwm_cb(void *old_value, const void *memory, uint16_t size, void *user_ptr, void *arg);


static struct {
    watcher_t watcher;
    uint8_t   relays;
    uint8_t   pwm;
} state = {0};


void observer_init(mut_model_t *model) {
    WATCHER_INIT_STD(&state.watcher, model);
    WATCHER_ADD_ENTRY(&state.watcher, &state.relays, relays_cb, model);
    WATCHER_ADD_ENTRY(&state.watcher, &state.pwm, pwm_cb, model);
}


void observer_manage(model_t *model) {
    state.relays = model_get_relay_map(model);
    state.pwm    = model_get_pwm_percentage(model);

    watcher_watch(&state.watcher, timestamp_get());
}


static void relays_cb(void *old_value, const void *memory, uint16_t size, void *user_ptr, void *arg) {
    (void)old_value;
    (void)memory;
    (void)size;
    (void)arg;

    model_t *model      = user_ptr;
    uint8_t  relays_map = model_get_relay_map(model);
    for (size_t i = 0; i < RELAYS_NUM; i++) {
        bsp_relays_set(i, (relays_map & (1 << i)) > 0);
    }
}


static void pwm_cb(void *old_value, const void *memory, uint16_t size, void *user_ptr, void *arg) {
    (void)old_value;
    (void)memory;
    (void)size;
    (void)arg;

    model_t *model = user_ptr;
    bsp_pwm_set(model_get_pwm_percentage(model));
}
