#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "core/bus.h"
#include "core/dma.h"
#include "core/ppu.h"

static uint8_t s_coreDmaRegisterSource;
static uint16_t s_coreDmaSource;
static int s_coreDmaDestinationIndex;
static bool s_coreDmaTransferring;

void coreDmaReset(void) {
    s_coreDmaTransferring = false;
}

void coreDmaCycle(void) {
    if(!s_coreDmaTransferring) {
        return;
    }

    corePpuWriteOamDma(s_coreDmaDestinationIndex++, coreBusReadDma(s_coreDmaSource++));

    if(s_coreDmaDestinationIndex == 160) {
        s_coreDmaTransferring = false;
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
    s_coreDmaTransferring = true;
    s_coreDmaDestinationIndex = 0;
}

bool coreDmaIsRunning(void) {
    return s_coreDmaTransferring;
}
