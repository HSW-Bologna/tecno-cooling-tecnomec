#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>


#define INPUTS_NUM 6
#define RELAYS_NUM 6


struct model {
    struct {
        uint16_t language;
    } config;

    struct {
        struct {
            uint8_t  inputs;
            uint16_t humidity;
            uint16_t pressure;
            int16_t  temperature;
        } sensors;

        uint8_t test_mode;
        int16_t tested_relay;
        int16_t test_pwm;
    } run;
};


typedef const struct model model_t;
typedef struct model       mut_model_t;


void    model_init(mut_model_t *model);
int     model_get_language(model_t *model);
uint8_t model_get_relay_map(model_t *model);
void    model_exit_test_mode(mut_model_t *model);
void    model_enter_test_mode(mut_model_t *model);
void    model_test_toggle_relay(mut_model_t *model, uint16_t relay);
uint8_t model_get_pwm_percentage(model_t *model);


#endif
