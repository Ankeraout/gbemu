#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "core/bus.h"
#include "core/dma.h"
#include "core/ppu.h"

static uint8_t s_coreDmaRegisterSource;
static unsigned int s_coreDmaStartCountdown;
static uint16_t s_coreDmaSource;
static unsigned int s_coreDmaDestination;
static uint8_t s_coreDmaBuffer;
static bool s_coreDmaBufferFilled;

void coreDmaReset(void) {
    s_coreDmaRegisterSource = 0;
    s_coreDmaSource = 0x00a0;
    s_coreDmaStartCountdown = 0;
    s_coreDmaBufferFilled = false;
}

void coreDmaCycle(void) {
    if(s_coreDmaStartCountdown != 0) {
        s_coreDmaStartCountdown--;

        if(s_coreDmaStartCountdown == 0) {
            s_coreDmaDestination = 0;

            if(s_coreDmaRegisterSource >= 0xe0) {
                s_coreDmaSource ^= 0x2000;
            }

            s_coreDmaBufferFilled = false;
        }
    }

    if(s_coreDmaBufferFilled) {
        corePpuWriteOamDma(s_coreDmaDestination, s_coreDmaBuffer);
        s_coreDmaBufferFilled = false;
    }

    if((s_coreDmaSource & 0xff) < 0xa0) {
        s_coreDmaBuffer = coreBusReadDma(s_coreDmaSource++);
        s_coreDmaBufferFilled = true;
    }
}

uint8_t coreDmaRead(uint16_t p_address) {
    M_UNUSED_PARAMETER(p_address);

    return s_coreDmaRegisterSource;
}

void coreDmaWrite(uint16_t p_address, uint8_t p_value) {
    M_UNUSED_PARAMETER(p_address);

    s_coreDmaRegisterSource = p_value;
    s_coreDmaSource = (p_value << 8);
    s_coreDmaStartCountdown = 2;
}

bool coreDmaIsRunning(void) {
    return s_coreDmaBufferFilled;
}
