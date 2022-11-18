#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "core/bus.h"
#include "core/cpu.h"

union tu_coreCpuRegisterPair {
    struct {
        uint8_t low;
        uint8_t high;
    } byte;

    uint16_t word;
};

static uint16_t s_coreCpuInterruptJumpTable[32] = {
    0x0000, 0x0040, 0x0048, 0x0040,
    0x0050, 0x0040, 0x0048, 0x0040,
    0x0058, 0x0040, 0x0048, 0x0040,
    0x0050, 0x0040, 0x0048, 0x0040,
    0x0060, 0x0040, 0x0048, 0x0040,
    0x0050, 0x0040, 0x0048, 0x0040,
    0x0058, 0x0040, 0x0048, 0x0040,
    0x0050, 0x0040, 0x0048, 0x0040
};

static uint16_t s_coreCpuInterruptFlagClearTable[32] = {
    0xff, 0xfe, 0xfd, 0xfe,
    0xfc, 0xfe, 0xfd, 0xfe,
    0xf8, 0xfe, 0xfd, 0xfe,
    0xfc, 0xfe, 0xfd, 0xfe,
    0xf0, 0xfe, 0xfd, 0xfe,
    0xfc, 0xfe, 0xfd, 0xfe,
    0xf8, 0xfe, 0xfd, 0xfe,
    0xfc, 0xfe, 0xfd, 0xfe
};

static union tu_coreCpuRegisterPair s_coreCpuRegisterAF;
static union tu_coreCpuRegisterPair s_coreCpuRegisterBC;
static union tu_coreCpuRegisterPair s_coreCpuRegisterDE;
static union tu_coreCpuRegisterPair s_coreCpuRegisterHL;
static bool s_coreCpuFlagZ;
static bool s_coreCpuFlagN;
static bool s_coreCpuFlagH;
static bool s_coreCpuFlagC;
static uint16_t s_coreCpuRegisterSP;
static uint16_t s_coreCpuRegisterPC;
static uint8_t s_coreCpuRegisterInterruptEnable;
static uint8_t s_coreCpuRegisterInterruptFlag;
static bool s_coreCpuCheckInterrupts;

static inline void coreCpuPrintState(void);
static inline uint16_t coreCpuPop(void);
static inline void coreCpuPush(uint16_t p_value);
static inline uint8_t coreCpuFetch8(void);
static inline uint16_t coreCpuFetch16(void);
static inline uint16_t coreCpuRead16(uint16_t p_address);
static inline void coreCpuWrite16(uint16_t p_address, uint16_t p_value);
static inline void coreCpuOpAdd(uint8_t p_value);
static inline void coreCpuOpAddHl(uint16_t p_value);
static inline void coreCpuOpDaa(void);
static inline uint8_t coreCpuOpDec8(uint8_t p_value);
static inline void coreCpuOpInc16(uint16_t *p_register);
static inline uint8_t coreCpuOpInc8(uint8_t p_value);
static inline void coreCpuOpRlBase(uint8_t *p_register);
static inline void coreCpuOpRlcBase(uint8_t *p_register);
static inline void coreCpuOpRrBase(uint8_t *p_register);
static inline void coreCpuOpRrcBase(uint8_t *p_register);

void coreCpuReset(void) {
    s_coreCpuRegisterAF.word = 0x0000;
    s_coreCpuRegisterBC.word = 0x0000;
    s_coreCpuRegisterDE.word = 0x0000;
    s_coreCpuRegisterHL.word = 0x0000;
    s_coreCpuFlagZ = false;
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = false;
    s_coreCpuRegisterSP = 0x0000;
    s_coreCpuRegisterPC = 0x0000;
    s_coreCpuRegisterInterruptEnable = 0x00;
    s_coreCpuRegisterInterruptFlag = 0x00;
    s_coreCpuCheckInterrupts = true;

    coreCpuPrintState();
}

void coreCpuStep(void) {
    if(s_coreCpuCheckInterrupts) {
        s_coreCpuCheckInterrupts = false;

        uint8_t l_interrupts =
            s_coreCpuRegisterInterruptEnable & s_coreCpuRegisterInterruptFlag;

        if(l_interrupts != 0) {
            coreBusCycle();
            coreBusCycle();
            coreCpuPush(s_coreCpuRegisterPC);
            s_coreCpuRegisterPC = s_coreCpuInterruptJumpTable[l_interrupts];
            s_coreCpuRegisterInterruptFlag &=
                s_coreCpuInterruptFlagClearTable[l_interrupts];
            coreBusCycle();

            coreCpuPrintState();

            return; // Consider interrupt jump routine as a step
        }
    }

    uint8_t l_opcode = coreBusRead(s_coreCpuRegisterPC++);

    switch(l_opcode) {
        case 0x00: // NOP
            break;

        case 0x01: // LD BC, u16
            s_coreCpuRegisterBC.word = coreCpuFetch16();
            break;

        case 0x02: // LD (BC), A
            coreBusWrite(
                s_coreCpuRegisterBC.word,
                s_coreCpuRegisterAF.byte.high
            );

            break;

        case 0x03: // INC BC
            s_coreCpuRegisterBC.word++;
            coreBusCycle();
            break;

        case 0x04: // INC B
            s_coreCpuRegisterBC.byte.high =
                coreCpuOpInc8(s_coreCpuRegisterBC.byte.high);

            break;

        case 0x05: // DEC B
            s_coreCpuRegisterBC.byte.high =
                coreCpuOpDec8(s_coreCpuRegisterBC.byte.high);
            break;

        case 0x06: // LD B, d8
            s_coreCpuRegisterBC.byte.high = coreCpuFetch8();
            break;

        case 0x07: // RLCA
            coreCpuOpRlcBase(&s_coreCpuRegisterAF.byte.high);
            s_coreCpuFlagZ = false;
            break;

        case 0x08: // LD (a16), SP
            coreCpuWrite16(coreCpuFetch16(), s_coreCpuRegisterSP);
            break;

        case 0x09: // ADD HL, BC
            coreCpuOpAddHl(s_coreCpuRegisterBC.word);
            break;

        case 0x0a: // LD A, (BC)
            s_coreCpuRegisterAF.byte.high =
                coreBusRead(s_coreCpuRegisterBC.word);

            break;

        case 0x0b: // DEC BC
            s_coreCpuRegisterBC.word--;
            coreBusCycle();
            break;

        case 0x0c: // INC C
            s_coreCpuRegisterBC.byte.low =
                coreCpuOpInc8(s_coreCpuRegisterBC.byte.low);

            break;

        case 0x0d: // DEC C
            s_coreCpuRegisterBC.byte.low =
                coreCpuOpDec8(s_coreCpuRegisterBC.byte.low);

            break;

        case 0x0e: // LD C, d8
            s_coreCpuRegisterBC.byte.low = coreCpuFetch8();
            break;

        case 0x0f: // RRCA
            coreCpuOpRrcBase(&s_coreCpuRegisterAF.byte.high);
            s_coreCpuFlagZ = false;
            break;

        case 0x10: // STOP
            // TODO
            break;

        case 0x11: // LD DE, d16
            s_coreCpuRegisterDE.word = coreCpuFetch16();
            break;

        case 0x12: // LD (DE), A
            coreBusWrite(
                s_coreCpuRegisterDE.word,
                s_coreCpuRegisterAF.byte.high
            );

            break;

        case 0x13: // INC DE
            s_coreCpuRegisterDE.word++;
            coreBusCycle();
            break;

        case 0x14: // INC D
            s_coreCpuRegisterDE.byte.high =
                coreCpuOpInc8(s_coreCpuRegisterDE.byte.high);

            break;

        case 0x15: // DEC D
            s_coreCpuRegisterDE.byte.high =
                coreCpuOpDec8(s_coreCpuRegisterDE.byte.high);

            break;

        case 0x16: // LD D, d8
            s_coreCpuRegisterDE.byte.high = coreCpuFetch8();
            break;

        case 0x17: // RLA
            coreCpuOpRlBase(&s_coreCpuRegisterAF.byte.high);
            s_coreCpuFlagZ = false;
            break;

        case 0x18: // JR r8
            s_coreCpuRegisterPC += (int8_t)coreCpuFetch8();
            coreBusCycle();
            break;

        case 0x19: // ADD HL, DE
            coreCpuOpAddHl(s_coreCpuRegisterDE.word);
            break;

        case 0x1a: // LD A, (DE)
            s_coreCpuRegisterAF.byte.high =
                coreCpuRead16(s_coreCpuRegisterDE.word);

            break;

        case 0x1b: // DEC DE
            s_coreCpuRegisterDE.word--;
            coreBusCycle();
            break;

        case 0x1c: // INC E
            s_coreCpuRegisterDE.byte.low =
                coreCpuOpInc8(s_coreCpuRegisterDE.byte.low);

            break;

        case 0x1d: // DEC E
            s_coreCpuRegisterDE.byte.low =
                coreCpuOpDec8(s_coreCpuRegisterDE.byte.low);

            break;

        case 0x1e: // LD E, d8
            s_coreCpuRegisterDE.byte.low = coreCpuFetch8();
            break;

        case 0x1f: // RRA
            coreCpuOpRrBase(&s_coreCpuRegisterAF.byte.high);
            s_coreCpuFlagZ = false;
            break;

        case 0x20: // JR NZ, r8
            if(s_coreCpuFlagZ) {
                coreBusCycle();
            } else {
                s_coreCpuRegisterPC += (int8_t)coreCpuFetch8();
                coreBusCycle();
            }

            break;

        case 0x21: // LD HL, d16
            s_coreCpuRegisterHL.word = coreCpuFetch16();
            break;

        case 0x22: // LD (HL+), A
            coreBusWrite(
                s_coreCpuRegisterHL.word++,
                s_coreCpuRegisterAF.byte.high
            );

            break;

        case 0x23: // INC HL
            s_coreCpuRegisterHL.word++;
            coreBusCycle();
            break;

        case 0x24: // INC H
            s_coreCpuRegisterHL.byte.high =
                coreCpuOpInc8(s_coreCpuRegisterHL.byte.high);

            break;

        case 0x25: // DEC H
            s_coreCpuRegisterHL.byte.high =
                coreCpuOpDec8(s_coreCpuRegisterHL.byte.high);

            break;

        case 0x26: // LD H, d8
            s_coreCpuRegisterHL.byte.high = coreCpuFetch8();
            break;

        case 0x27: // DAA
            coreCpuOpDaa();
            break;

        case 0x28: // JR Z, r8
            if(s_coreCpuFlagZ) {
                s_coreCpuRegisterPC += (int8_t)coreCpuFetch8();
                coreBusCycle();
            } else {
                coreBusCycle();
            }

            break;

        case 0x29: // ADD HL, HL
            coreCpuOpAddHl(s_coreCpuRegisterHL.word);
            break;

        case 0x2a: // LD A, (HL+)
            s_coreCpuRegisterAF.byte.high =
                coreBusRead(s_coreCpuRegisterHL.word++);

            break;

        case 0x2b: // DEC HL
            s_coreCpuRegisterHL.word--;
            coreBusCycle();
            break;

        case 0x2c: // INC L
            s_coreCpuRegisterHL.byte.low =
                coreCpuOpInc8(s_coreCpuRegisterHL.byte.low);

            break;

        case 0x2d: // DEC L
            s_coreCpuRegisterHL.byte.low =
                coreCpuOpDec8(s_coreCpuRegisterHL.byte.low);

            break;

        case 0x2e: // LD L, d8
            s_coreCpuRegisterHL.byte.low = coreCpuFetch8();
            break;

        case 0x2f: // CPL
            s_coreCpuFlagN = true;
            s_coreCpuFlagH = true;
            break;

        case 0x30: // JR NC, r8
            if(s_coreCpuFlagC) {
                coreBusCycle();
            } else {
                s_coreCpuRegisterPC += (int8_t)coreCpuFetch8();
                coreBusCycle();
            }

            break;

        case 0x31: // LD SP, d16
            s_coreCpuRegisterSP = coreCpuFetch16();
            break;

        case 0x32: // LD (HL-), A
            coreBusWrite(
                s_coreCpuRegisterHL.word--,
                s_coreCpuRegisterAF.byte.high
            );

            break;

        case 0x33: // INC SP
            s_coreCpuRegisterSP++;
            coreBusCycle();
            break;

        case 0x34: // INC (HL)
            coreBusWrite(
                s_coreCpuRegisterHL.word,
                coreCpuOpInc8(coreBusRead(s_coreCpuRegisterHL.word))
            );

            break;

        case 0x35: // DEC (HL)
            coreBusWrite(
                s_coreCpuRegisterHL.word,
                coreCpuOpDec8(coreBusRead(s_coreCpuRegisterHL.word))
            );

            break;

        case 0x36: // LD (HL), d8
            coreBusWrite(s_coreCpuRegisterHL.word, coreCpuFetch8());
            break;

        case 0x37: // SCF
            s_coreCpuFlagN = false;
            s_coreCpuFlagH = false;
            s_coreCpuFlagC = true;
            break;

        case 0x38: // JR C, r8
            if(s_coreCpuFlagC) {
                s_coreCpuRegisterPC += (int8_t)coreCpuFetch8();
                coreBusCycle();
            } else {
                coreBusCycle();
            }

            break;

        case 0x39: // ADD HL, SP
            coreCpuOpAddHl(s_coreCpuRegisterSP);
            break;

        case 0x3a: // LD A, (HL-)
            s_coreCpuRegisterAF.byte.high =
                coreBusRead(s_coreCpuRegisterHL.word--);

            break;

        case 0x3b: // DEC SP
            s_coreCpuRegisterSP--;
            coreBusCycle();
            break;

        case 0x3c: // INC A
            s_coreCpuRegisterAF.byte.high =
                coreCpuOpInc8(s_coreCpuRegisterAF.byte.high);

            break;

        case 0x3d: // DEC A
            s_coreCpuRegisterAF.byte.high =
                coreCpuOpDec8(s_coreCpuRegisterAF.byte.high);

            break;

        case 0x3e: // LD A, d8
            s_coreCpuRegisterAF.byte.high = coreCpuFetch8();
            break;

        case 0x3f: // CCF
            s_coreCpuFlagN = false;
            s_coreCpuFlagH = false;
            s_coreCpuFlagC = !s_coreCpuFlagC;
            break;

        default: // Freeze the CPU
            // TODO
            break;
    }

    coreCpuPrintState();
}

uint8_t coreCpuRead(uint16_t p_address) {
    if(p_address == 0xff0f) { // IF
        return s_coreCpuRegisterInterruptFlag;
    } else if(p_address == 0xffff) { // IE
        return s_coreCpuRegisterInterruptEnable;
    } else {
        return 0xff;
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

static inline void coreCpuPrintState(void) {
    printf(
        "PC=0x%04x AF=0x%04x BC=0x%04x DE=0x%04x HL=0x%04x SP=0x%04x\n",
        s_coreCpuRegisterPC,
        s_coreCpuRegisterAF.word,
        s_coreCpuRegisterBC.word,
        s_coreCpuRegisterDE.word,
        s_coreCpuRegisterHL.word,
        s_coreCpuRegisterSP
    );
}

static inline uint16_t coreCpuPop(void) {
    uint16_t l_result;

    l_result = coreBusRead(s_coreCpuRegisterSP++);
    l_result |= coreBusRead(s_coreCpuRegisterSP++) << 8;

    return l_result;
}

static inline void coreCpuPush(uint16_t p_value) {
    coreBusWrite(s_coreCpuRegisterSP--, p_value);
    coreBusWrite(s_coreCpuRegisterSP--, p_value >> 8);
}

static inline uint8_t coreCpuFetch8(void) {
    return coreBusRead(s_coreCpuRegisterPC++);
}

static inline uint16_t coreCpuFetch16(void) {
    uint16_t l_returnValue;

    l_returnValue = coreBusRead(s_coreCpuRegisterPC++);
    l_returnValue |= coreBusRead(s_coreCpuRegisterPC++) << 8;

    return l_returnValue;
}

static inline uint16_t coreCpuRead16(uint16_t p_address) {
    uint16_t l_returnValue;

    l_returnValue = coreBusRead(p_address);
    l_returnValue |= coreBusRead(p_address + 1) << 8;

    return l_returnValue;
}

static inline void coreCpuWrite16(uint16_t p_address, uint16_t p_value) {
    coreBusWrite(p_address, p_value);
    coreBusWrite(p_address + 1, p_value >> 8);
}

static inline void coreCpuOpAdd(uint8_t p_value) {
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = (
        (s_coreCpuRegisterAF.byte.high & 0x0f) + (p_value & 0x0f)
    ) >= 0x10;
    s_coreCpuFlagC = (s_coreCpuRegisterAF.byte.high + p_value) >= 0x100;

    s_coreCpuRegisterAF.byte.high += p_value;

    s_coreCpuFlagZ = s_coreCpuRegisterAF.byte.high == 0;
}

static inline void coreCpuOpAddHl(uint16_t p_value) {
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = (
        (s_coreCpuRegisterHL.word & 0x0fff)
        + (p_value & 0x0fff)
    ) >= 0x1000;
    s_coreCpuFlagC = (s_coreCpuRegisterHL.word + p_value) >= 0x10000;

    s_coreCpuRegisterHL.word += p_value;

    coreBusCycle();
}

static inline void coreCpuOpDaa(void) {
    if(s_coreCpuFlagN) {
        if(s_coreCpuFlagC) {
            s_coreCpuRegisterAF.byte.high -= 0x60;
        }

        if(s_coreCpuFlagH) {
            s_coreCpuRegisterAF.byte.high -= 0x06;
        }
    } else {
        if(s_coreCpuFlagC || (s_coreCpuRegisterAF.byte.high > 0x99)) {
            s_coreCpuRegisterAF.byte.high += 0x60;
            s_coreCpuFlagC = true;
        }

        if(s_coreCpuFlagH || ((s_coreCpuRegisterAF.byte.high & 0x0f) > 0x09)) {
            s_coreCpuRegisterAF.byte.high += 0x06;
        }
    }

    s_coreCpuFlagZ = s_coreCpuRegisterAF.byte.high == 0;
    s_coreCpuFlagH = false;
}

static inline uint8_t coreCpuOpDec8(uint8_t p_value) {
    p_value--;

    s_coreCpuFlagZ = p_value == 0;
    s_coreCpuFlagN = true;
    s_coreCpuFlagH = (p_value & 0x0f) == 0;

    return p_value;
}

static inline void coreCpuOpInc16(uint16_t *p_register) {
    coreBusCycle();
    (*p_register)++;
}

static inline uint8_t coreCpuOpInc8(uint8_t p_value) {
    p_value++;

    s_coreCpuFlagZ = p_value == 0;
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = (p_value & 0x0f) == 0;

    return p_value;
}

static inline void coreCpuOpRlBase(uint8_t *p_register) {
    uint8_t l_savedBit = s_coreCpuFlagC ? 1 : 0;

    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = (*p_register & 0x80) != 0;

    *p_register =  (*p_register << 1) | l_savedBit;
}

static inline void coreCpuOpRlcBase(uint8_t *p_register) {
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = *p_register >> 7;

    *p_register = (*p_register << 1) | (s_coreCpuFlagC ? 1 : 0);
}

static inline void coreCpuOpRrBase(uint8_t *p_register) {
    uint8_t l_savedBit = s_coreCpuFlagC ? 0x80 : 0;

    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = (*p_register & 0x01) != 0;

    *p_register =  (*p_register >> 1) | l_savedBit;
}

static inline void coreCpuOpRrcBase(uint8_t *p_register) {
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = (*p_register & 0x01) != 0;

    *p_register = (*p_register >> 1) | (s_coreCpuFlagC ? 0x80 : 0x00);
}
