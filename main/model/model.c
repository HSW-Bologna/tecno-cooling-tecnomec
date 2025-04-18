#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "model.h"


void model_init(mut_model_t *model) {
    assert(model != NULL);

    memset(model, 0, sizeof(*model));

    model->run.tested_relay = -1;
}


int model_get_language(model_t *model) {
    assert(model != NULL);

    return 0;
}


void model_enter_test_mode(mut_model_t *model) {
    assert(model != NULL);

    model->run.test_mode    = 1;
    model->run.tested_relay = -1;
    model->run.test_pwm     = 0;
}


void model_exit_test_mode(mut_model_t *model) {
    assert(model != NULL);

    model->run.test_mode    = 0;
    model->run.tested_relay = -1;
    model->run.test_pwm     = 0;
}


uint8_t model_get_relay_map(model_t *model) {
    assert(model != NULL);

    if (model->run.test_mode) {
        if (model->run.tested_relay >= 0) {
            return (1 << model->run.tested_relay);
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}


uint8_t model_get_pwm_percentage(model_t *model) {
    assert(model != NULL);

    if (model->run.test_mode) {
        return model->run.test_pwm;
    } else {
        return 0;
    }
}


void model_test_toggle_relay(mut_model_t *model, uint16_t relay) {
    assert(model != NULL);

    if (model->run.tested_relay == relay) {
        model->run.tested_relay = -1;
    } else {
        model->run.tested_relay = relay;
    }
}
