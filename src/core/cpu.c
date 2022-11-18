#include <stdbool.h>
#include <stdint.h>

#include "core/bus.h"
#include "core/cpu.h"

union tu_coreCpuRegisterPair {
    struct {
        uint8_t low;
        uint8_t high;
    } byte;

    uint16_t word;
};

static union tu_coreCpuRegisterPair s_coreCpuRegisterAF;
static union tu_coreCpuRegisterPair s_coreCpuRegisterBC;
static union tu_coreCpuRegisterPair s_coreCpuRegisterDE;
static union tu_coreCpuRegisterPair s_coreCpuRegisterHL;
static uint16_t s_coreCpuRegisterSP;
static uint16_t s_coreCpuRegisterPC;
static uint8_t s_coreCpuRegisterInterruptEnable;
static uint8_t s_coreCpuRegisterInterruptFlag;
static bool s_coreCpuCheckInterrupts;

static uint16_t coreCpuPop(void);
static void coreCpuPush(uint16_t p_value);

void coreCpuReset(void) {
    s_coreCpuRegisterAF.word = 0x0000;
    s_coreCpuRegisterBC.word = 0x0000;
    s_coreCpuRegisterDE.word = 0x0000;
    s_coreCpuRegisterHL.word = 0x0000;
    s_coreCpuRegisterSP = 0x0000;
    s_coreCpuRegisterPC = 0x0000;
    s_coreCpuRegisterInterruptEnable = 0x00;
    s_coreCpuRegisterInterruptFlag = 0x00;
    s_coreCpuCheckInterrupts = true;
}

void coreCpuStep(void) {
    if(s_coreCpuCheckInterrupts) {
        uint8_t l_interrupts =
            s_coreCpuRegisterInterruptEnable & s_coreCpuRegisterInterruptFlag;

        if(l_interrupts != 0) {
            for(int l_shift = 0; l_shift < 5; l_shift++) {
                uint8_t l_mask = 1 << l_shift;

                if((l_interrupts & l_mask) != 0) {
                    coreBusCycle();
                    coreBusCycle();
                    coreCpuPush(s_coreCpuRegisterPC);
                    s_coreCpuRegisterPC = 0x0040 | (0x08 * l_shift);
                    coreBusCycle();

                    break;
                }
            }
        }

        s_coreCpuCheckInterrupts = false;
    }

    uint8_t l_opcode = coreBusRead(s_coreCpuRegisterPC++);
}

uint8_t coreCpuRead(uint16_t p_address) {
    if(p_address == 0xff0f) { // IF
        return s_coreCpuRegisterInterruptFlag;
    } else if(p_address == 0xffff) { // IE
        return s_coreCpuRegisterInterruptEnable;
    }
}

void coreCpuWrite(uint16_t p_address, uint8_t p_value) {
    if(p_address == 0xff0f) { // IF
        s_coreCpuRegisterInterruptFlag = p_value & 0x1f;
    } else if(p_address == 0xffff) { // IE
        s_coreCpuRegisterInterruptEnable = p_value & 0x1f;
    }

    s_coreCpuCheckInterrupts = true;
}

static uint16_t coreCpuPop(void) {
    uint16_t l_result;

    l_result = coreBusRead(s_coreCpuRegisterSP++);
    l_result |= coreBusRead(s_coreCpuRegisterSP++) << 8;

    return l_result;
}

static void coreCpuPush(uint16_t p_value) {
    coreBusWrite(s_coreCpuRegisterSP--, s_coreCpuRegisterPC);
    coreBusWrite(s_coreCpuRegisterSP--, s_coreCpuRegisterPC >> 8);
}
