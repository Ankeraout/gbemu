#include <stdint.h>

#include "core/ppu.h"

#define C_PPU_VRAM_SIZE 8192
#define C_PPU_OAM_SIZE 256

static uint8_t s_corePpuVramData[C_PPU_VRAM_SIZE];
static uint8_t s_corePpuOamData[C_PPU_OAM_SIZE];

void corePpuReset(void) {
    for(int l_vramOffset = 0; l_vramOffset < C_PPU_VRAM_SIZE; l_vramOffset++) {
        s_corePpuVramData[l_vramOffset] = 0;
    }

    for(int l_oamOffset = 0; l_oamOffset < C_PPU_OAM_SIZE; l_oamOffset++) {
        s_corePpuOamData[l_oamOffset] = 0;
    }
}

uint8_t corePpuReadIo(uint16_t p_address) {

}

uint8_t corePpuReadVram(uint16_t p_address) {
    return s_corePpuVramData[p_address & 0x1fff];
}

uint8_t corePpuReadOam(uint16_t p_address) {
    return s_corePpuOamData[p_address & 0x00ff];
}

void corePpuWriteIo(uint16_t p_address, uint8_t p_value) {

}

void corePpuWriteVram(uint16_t p_address, uint8_t p_value) {
    s_corePpuVramData[p_address & 0x1fff] = p_value;
}

void corePpuWriteOam(uint16_t p_address, uint8_t p_value) {
    s_corePpuOamData[p_address & 0x00ff] = p_value;
}
