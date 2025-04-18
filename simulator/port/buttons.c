#include <SDL2/SDL.h>
#include "bsp/buttons.h"
#include "services/timestamp.h"


static keypad_key_t keys[] = {
    KEYPAD_KEY(0x01, BSP_BUTTON_1), KEYPAD_KEY(0x02, BSP_BUTTON_2), KEYPAD_KEY(0x04, BSP_BUTTON_3),
    KEYPAD_KEY(0x08, BSP_BUTTON_4), KEYPAD_KEY(0x10, BSP_BUTTON_5), KEYPAD_KEY(0x20, BSP_BUTTON_6),
    KEYPAD_KEY(0x40, BSP_BUTTON_7), KEYPAD_KEY(0x80, BSP_BUTTON_8), KEYPAD_NULL_KEY,
};


keypad_event_t bsp_button_manage(void) {
    static const uint8_t *keystates = NULL;
    unsigned int          input     = 0;

    if (keystates == NULL) {
        keystates = SDL_GetKeyboardState(NULL);
    }

    SDL_PumpEvents();
    if (keystates[SDL_SCANCODE_1] || keystates[SDL_SCANCODE_KP_1]) {
        input |= 0x40;
    } else {
        input &= ~0x40;
    }
    if (keystates[SDL_SCANCODE_2] || keystates[SDL_SCANCODE_KP_2]) {
        input |= 0x80;
    } else {
        input &= ~0x80;
    }
    if (keystates[SDL_SCANCODE_4] || keystates[SDL_SCANCODE_KP_4]) {
        input |= 0x08;
    } else {
        input &= ~0x08;
    }
    if (keystates[SDL_SCANCODE_5] || keystates[SDL_SCANCODE_KP_5]) {
        input |= 0x10;
    } else {
        input &= ~0x10;
    }
    if (keystates[SDL_SCANCODE_6] || keystates[SDL_SCANCODE_KP_6]) {
        input |= 0x20;
    } else {
        input &= ~0x20;
    }
    if (keystates[SDL_SCANCODE_7] || keystates[SDL_SCANCODE_KP_7]) {
        input |= 0x1;
    } else {
        input &= ~0x1;
    }
    if (keystates[SDL_SCANCODE_8] || keystates[SDL_SCANCODE_KP_8]) {
        input |= 0x2;
    } else {
        input &= ~0x2;
    }
    if (keystates[SDL_SCANCODE_9] || keystates[SDL_SCANCODE_KP_9]) {
        input |= 0x4;
    } else {
        input &= ~0x4;
    }

    return keypad_routine(keys, 40, 1500, 100, timestamp_get(), input);
}
