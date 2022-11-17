#ifndef __INCLUDE_CORE_BIOS_H__
#define __INCLUDE_CORE_BIOS_H__

#include <stdint.h>

void coreBiosSetBios(const void *p_bios);
uint8_t coreBiosRead(uint16_t p_address);

#endif
