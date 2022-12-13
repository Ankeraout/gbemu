#ifndef __INCLUDE_CORE_JOYPAD_H__
#define __INCLUDE_CORE_JOYPAD_H__

#include <stdbool.h>
#include <stdint.h>

enum te_coreJoypadInput {
    E_JOYPADINPUT_RIGHT,
    E_JOYPADINPUT_LEFT,
    E_JOYPADINPUT_UP,
    E_JOYPADINPUT_DOWN,
    E_JOYPADINPUT_A,
    E_JOYPADINPUT_B,
    E_JOYPADINPUT_SELECT,
    E_JOYPADINPUT_START
};

void coreJoypadReset(void);
uint8_t coreJoypadRead(uint16_t p_address);
void coreJoypadWrite(uint16_t p_address, uint8_t p_value);
void coreJoypadSetInput(enum te_coreJoypadInput p_input, bool p_pressed);

#endif
