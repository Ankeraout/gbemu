#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "core/joypad.h"

static bool s_coreJoypadSelectAction;
static bool s_coreJoypadSelectDirection;
static uint8_t s_coreJoypadRegisterJoyp;
static bool s_coreJoypadButtonState[8];

static void coreJoypadUpdate(void);

void coreJoypadReset(void) {
    s_coreJoypadRegisterJoyp = 0xff;
    s_coreJoypadSelectAction = false;
    s_coreJoypadSelectDirection = false;
}

uint8_t coreJoypadRead(uint16_t p_address) {
    M_UNUSED_PARAMETER(p_address);

    return s_coreJoypadRegisterJoyp;
}

void coreJoypadWrite(uint16_t p_address, uint8_t p_value) {
    M_UNUSED_PARAMETER(p_address);

    s_coreJoypadSelectAction = (p_value & 0x20) == 0;
    s_coreJoypadSelectDirection = (p_value & 0x10) == 0;

    coreJoypadUpdate();
}

void coreJoypadSetInput(enum te_coreJoypadInput p_input, bool p_pressed) {
    s_coreJoypadButtonState[p_input] = p_pressed;

    coreJoypadUpdate();
}

static void coreJoypadUpdate(void) {
    s_coreJoypadRegisterJoyp = 0xff;

    if(s_coreJoypadSelectAction) {
        s_coreJoypadRegisterJoyp &= 0xdf;

        int l_shift = 0;

        for(int l_button = 4; l_button < 8; l_button++) {
            if(s_coreJoypadButtonState[l_button]) {
                s_coreJoypadRegisterJoyp &= ~(1 << l_shift);
            }

            l_shift++;
        }
    }

    if(s_coreJoypadSelectDirection) {
        s_coreJoypadRegisterJoyp &= 0xef;

        for(int l_button = 0; l_button < 4; l_button++) {
            if(s_coreJoypadButtonState[l_button]) {
                s_coreJoypadRegisterJoyp &= ~(1 << l_button);
            }
        }
    }
}
