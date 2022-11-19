#include <stdint.h>

#include "core/wram.h"

#define C_WRAM_SIZE 8192

static uint8_t s_coreWramData[C_WRAM_SIZE];

void coreWramReset(void) {
    for(int l_wramOffset = 0; l_wramOffset < C_WRAM_SIZE; l_wramOffset++) {
        s_coreWramData[l_wramOffset] = 0;
    }
}

uint8_t coreWramRead(uint16_t p_address) {
    return s_coreWramData[p_address & 0x1fff];
}

void coreWramWrite(uint16_t p_address, uint8_t p_value) {
    s_coreWramData[p_address & 0x1fff] = p_value;
}
