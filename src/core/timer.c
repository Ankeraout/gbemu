#include <stdbool.h>
#include <stdint.h>

#include "core/apu.h"
#include "core/cpu.h"
#include "core/timer.h"

static uint16_t s_coreTimerRegisterDiv;
static uint8_t s_coreTimerRegisterTima;
static uint8_t s_coreTimerRegisterTma;
static uint8_t s_coreTimerRegisterTac;
static int s_coreTimerClockSelect;
static int s_coreTimerClockBit;
static bool s_coreTimerEnable;
static bool s_coreTimerReloadTima;
static bool s_coreTimerReloadTima2;

static const int s_coreTimerInputClockSelectValues[4] = {
    1 << 10,
    1 << 4,
    1 << 6,
    1 << 8
};

static const int s_coreTimerInputClockBitValues[4] = {
    1 << 9,
    1 << 3,
    1 << 5,
    1 << 7
};

static inline void coreTimerIncreaseTima(void);
static inline void coreTimerUpdateDiv(uint16_t p_newValue);

void coreTimerReset(void) {
    s_coreTimerRegisterDiv = 0;
    s_coreTimerRegisterTima = 0;
    s_coreTimerRegisterTma = 0;
    s_coreTimerRegisterTac = 0;
    s_coreTimerClockSelect = s_coreTimerInputClockSelectValues[0];
    s_coreTimerClockBit = s_coreTimerInputClockBitValues[0];
    s_coreTimerEnable = false;
    s_coreTimerReloadTima = false;
    s_coreTimerReloadTima2 = false;
}

void coreTimerCycle(void) {
    coreTimerUpdateDiv(s_coreTimerRegisterDiv + 4);
}

uint8_t coreTimerRead(uint16_t p_address) {
    switch(p_address) {
        case 0xff04: return s_coreTimerRegisterDiv >> 8;
        case 0xff05:
            if(s_coreTimerReloadTima) {
                return 0x00;
            } else {
                return s_coreTimerRegisterTima;
            }

        case 0xff06: return s_coreTimerRegisterTma;
        case 0xff07: return s_coreTimerRegisterTac;
        default: return 0xff;
    }
}

void coreTimerWrite(uint16_t p_address, uint8_t p_value) {
    switch(p_address) {
        case 0xff04: coreTimerUpdateDiv(0); break;
        case 0xff05:
            if(!s_coreTimerReloadTima2) {
                s_coreTimerRegisterTima = p_value;

                if(s_coreTimerReloadTima) {
                    s_coreTimerReloadTima = false;
                }
            }

            break;

        case 0xff06:
            s_coreTimerRegisterTma = p_value;

            if(s_coreTimerReloadTima2) {
                s_coreTimerRegisterTima = p_value;
            }

            break;

        case 0xff07:
            {
                int l_oldBit = s_coreTimerClockBit;
                bool l_oldEnabled = s_coreTimerEnable;

                s_coreTimerEnable = (p_value & 0x04) != 0;
                s_coreTimerClockBit = s_coreTimerInputClockBitValues[p_value & 0x03];
                s_coreTimerClockSelect = s_coreTimerInputClockSelectValues[p_value & 0x03];
                s_coreTimerRegisterTac = p_value | 0xf8;

                if(l_oldEnabled) {
                    if(!s_coreTimerEnable) {
                        if((s_coreTimerRegisterDiv & s_coreTimerClockBit) != 0) {
                            coreTimerIncreaseTima();
                        }
                    }
                } else {
                    bool l_glitch = (s_coreTimerRegisterDiv & l_oldBit) != 0;

                    if(s_coreTimerEnable) {
                        l_glitch &= (s_coreTimerRegisterDiv & s_coreTimerClockBit) == 0;
                    }

                    if(l_glitch) {
                        coreTimerIncreaseTima();
                    }
                }
            }

        default:
            break;
    }
}

static inline void coreTimerIncreaseTima(void) {
    s_coreTimerRegisterTima++;

    if(s_coreTimerRegisterTima == 0) {
        s_coreTimerReloadTima = true;
    }
}

static inline void coreTimerUpdateDiv(uint16_t p_newValue) {
    bool l_oldBit = (s_coreTimerRegisterDiv & s_coreTimerClockBit) != 0;
    bool l_newBit = (p_newValue & s_coreTimerClockBit) != 0;
    bool l_oldDivApuBit = (s_coreTimerRegisterDiv & (1 << 12)) != 0;
    bool l_newDivApuBit = (p_newValue & (1 << 12)) != 0;

    s_coreTimerRegisterDiv = p_newValue;
    s_coreTimerReloadTima2 = false;

    if(s_coreTimerReloadTima) {
        s_coreTimerReloadTima = false;
        s_coreTimerRegisterTima = s_coreTimerRegisterTma;
        s_coreTimerReloadTima2 = true;
        coreCpuRequestInterrupt(E_CPUINTERRUPT_TIMER);
    }

    if(s_coreTimerEnable) {
        if(l_oldBit && !l_newBit) {
            coreTimerIncreaseTima();
        }
    }

    if(l_oldDivApuBit & !l_newDivApuBit) {
        coreApuDivCycle();
    }
}
