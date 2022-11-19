#include <stdint.h>

#include "core/hram.h"

#define C_HRAM_SIZE 127

static uint8_t s_coreHramData[C_HRAM_SIZE];

void coreHramReset(void) {
    for(int l_hramOffset = 0; l_hramOffset < C_HRAM_SIZE; l_hramOffset++) {
        s_coreHramData[l_hramOffset] = 0;
    }
}

uint8_t coreHramRead(uint16_t p_address) {
    return s_coreHramData[p_address & 0x007f];
}

void coreHramWrite(uint16_t p_address, uint8_t p_value) {
    s_coreHramData[p_address & 0x007f] = p_value;
}
