#ifndef __INCLUDE_CORE_JOYPAD_H__
#define __INCLUDE_CORE_JOYPAD_H__

#include <stdint.h>

void coreJoypadReset(void);
uint8_t coreJoypadRead(uint16_t p_address);
void coreJoypadWrite(uint16_t p_address, uint8_t p_value);

#endif
