#include <stdint.h>

#include "core/bios.h"

static const uint8_t *s_coreBiosBuffer;

void coreBiosSetBios(const void *p_bios) {
    s_coreBiosBuffer = p_bios;
}

uint8_t coreBiosRead(uint16_t p_address) {
    return s_coreBiosBuffer[p_address & 0x00ff];
}
