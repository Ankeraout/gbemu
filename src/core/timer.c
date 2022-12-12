#include <stdbool.h>
#include <stdint.h>

#include "core/cpu.h"
#include "core/timer.h"

static uint16_t s_coreTimerRegisterDiv;
static uint8_t s_coreTimerRegisterTima;
static uint8_t s_coreTimerRegisterTma;
static uint8_t s_coreTimerRegisterTac;
static int s_coreTimerClockDivisor;
static int s_coreTimerClockDivisorCounter;
static bool s_coreTimerEnable;

static const int s_coreTimerClockDivisorTable[4] = {
    0x3ff,
    0xf,
    0x3f,
    0xff
};

void coreTimerReset(void) {
    s_coreTimerRegisterDiv = 0;
    s_coreTimerRegisterTima = 0;
    s_coreTimerRegisterTma = 0;
    s_coreTimerRegisterTac = 0;
    s_coreTimerClockDivisor = 1024;
    s_coreTimerClockDivisorCounter = 0;
    s_coreTimerEnable = false;
}

void coreTimerCycle(void) {
    s_coreTimerRegisterDiv += 4;
    s_coreTimerClockDivisorCounter += 4;

    if(s_coreTimerEnable && ((s_coreTimerClockDivisorCounter & s_coreTimerClockDivisor) == 0)) {
        s_coreTimerRegisterTima++;

        if(s_coreTimerRegisterTima == 0) {
            s_coreTimerRegisterTima = s_coreTimerRegisterTma;
            coreCpuRequestInterrupt(E_CPUINTERRUPT_TIMER);
        }
    }
}

uint8_t coreTimerRead(uint16_t p_address) {
    if(p_address == 0xff04) {
        return s_coreTimerRegisterDiv >> 8;
    } else if(p_address == 0xff05) {
        return s_coreTimerRegisterTima;
    } else if(p_address == 0xff06) {
        return s_coreTimerRegisterTma;
    } else if(p_address == 0xff07) {
        return s_coreTimerRegisterTac;
    }
}

void coreTimerWrite(uint16_t p_address, uint8_t p_value) {
    if(p_address == 0xff04) {
        s_coreTimerRegisterDiv = 0;
    } else if(p_address == 0xff05) {
        s_coreTimerRegisterTima = p_value;
    } else if(p_address == 0xff06) {
        s_coreTimerRegisterTma = p_value;
    } else if(p_address == 0xff07) {
        s_coreTimerRegisterTac = p_value;

        s_coreTimerEnable = (p_value & 0x04) != 0;
        s_coreTimerClockDivisor = s_coreTimerClockDivisorTable[p_value & 0x03];
    }
}
