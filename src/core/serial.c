#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "core/cpu.h"
#include "core/serial.h"

static uint8_t s_coreSerialRegisterSb;
static uint8_t s_coreSerialRegisterSc;
static bool s_coreSerialTransferring;
static bool s_coreSerialInternalClock;
static int s_coreSerialTransferCounter;
static int s_coreSerialClock;

void coreSerialReset(void) {
    s_coreSerialRegisterSb = 0xff;
    s_coreSerialRegisterSc = 0xef;
    s_coreSerialTransferring = false;
    s_coreSerialInternalClock = false;
    s_coreSerialTransferCounter = 0;
}

void coreSerialCycle(void) {
    s_coreSerialClock++;

    if((s_coreSerialClock & 0x7f) == 0) {
        if(s_coreSerialTransferring && !s_coreSerialInternalClock) {
            s_coreSerialRegisterSb <<= 1;
            s_coreSerialRegisterSb |= 0x01;
            s_coreSerialTransferCounter++;

            if(s_coreSerialTransferCounter == 8) {
                s_coreSerialRegisterSc &= 0x7f;
                s_coreSerialTransferring = false;

                coreCpuRequestInterrupt(E_CPUINTERRUPT_SERIAL);
            }
        }
    }
}

uint8_t coreSerialRead(uint16_t p_address) {
    if(p_address == 0xff01) {
        return s_coreSerialRegisterSb;
    } else {
        return s_coreSerialRegisterSc;
    }
}

void coreSerialWrite(uint16_t p_address, uint8_t p_value) {
    if(p_address == 0xff01) {
        s_coreSerialRegisterSb = p_value;
    } else {
        uint8_t l_oldSc = s_coreSerialRegisterSc;

        if(((~l_oldSc & p_value) & 0x80) != 0) {
            s_coreSerialTransferring = true;
            s_coreSerialTransferCounter = 0;
        }

        s_coreSerialInternalClock = (p_value & 0x01) != 0;

        s_coreSerialRegisterSc = p_value;
    }
}
