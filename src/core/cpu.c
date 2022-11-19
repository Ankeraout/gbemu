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
static bool s_coreCpuRegisterInterruptMasterEnable;
static bool s_coreCpuRegisterInterruptMasterEnableNextCycle;
static bool s_coreCpuCheckInterrupts;

static inline void coreCpuPrintState(void);
static inline uint16_t coreCpuPop(void);
static inline void coreCpuPush(uint16_t p_value);
static inline uint8_t coreCpuFetch8(void);
static inline uint16_t coreCpuFetch16(void);
static inline uint16_t coreCpuRead16(uint16_t p_address);
static inline void coreCpuWrite16(uint16_t p_address, uint16_t p_value);
static inline void coreCpuOpAdc(uint8_t p_value);
static inline void coreCpuOpAdd(uint8_t p_value);
static inline void coreCpuOpAddHl(uint16_t p_value);
static inline void coreCpuOpAnd(uint8_t p_value);
static inline void coreCpuOpCp(uint8_t p_value);
static inline void coreCpuOpDaa(void);
static inline uint8_t coreCpuOpDec8(uint8_t p_value);
static inline void coreCpuOpInc16(uint16_t *p_register);
static inline uint8_t coreCpuOpInc8(uint8_t p_value);
static inline void coreCpuOpOr(uint8_t p_value);
static inline uint8_t coreCpuOpRl(uint8_t p_value);
static inline uint8_t coreCpuOpRlBase(uint8_t p_value);
static inline uint8_t coreCpuOpRlc(uint8_t p_value);
static inline uint8_t coreCpuOpRlcBase(uint8_t p_value);
static inline uint8_t coreCpuOpRrBase(uint8_t p_value);
static inline uint8_t coreCpuOpRr(uint8_t p_value);
static inline uint8_t coreCpuOpRrc(uint8_t p_value);
static inline uint8_t coreCpuOpRrcBase(uint8_t p_value);
static inline void coreCpuOpSbc(uint8_t p_value);
static inline uint8_t coreCpuOpSla(uint8_t p_value);
static inline uint8_t coreCpuOpSra(uint8_t p_value);
static inline uint8_t coreCpuOpSrl(uint8_t p_value);
static inline void coreCpuOpSub(uint8_t p_value);
static inline uint8_t coreCpuOpSwap(uint8_t p_value);
static inline void coreCpuOpXor(uint8_t p_value);

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
    s_coreCpuRegisterInterruptMasterEnable = false;
    s_coreCpuRegisterInterruptMasterEnableNextCycle = false;
    s_coreCpuCheckInterrupts = true;

    coreCpuPrintState();
}

void coreCpuStep(void) {
    if(s_coreCpuCheckInterrupts) {
        s_coreCpuCheckInterrupts = false;

        if(s_coreCpuRegisterInterruptMasterEnableNextCycle) {
            s_coreCpuRegisterInterruptMasterEnable = true;
            s_coreCpuRegisterInterruptMasterEnableNextCycle = false;
            s_coreCpuCheckInterrupts = true;
        }

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

    switch(coreCpuFetch8()) {
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
            s_coreCpuRegisterAF.byte.high = coreCpuOpRlcBase(s_coreCpuRegisterAF.byte.high);
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
            s_coreCpuRegisterAF.byte.high = coreCpuOpRrcBase(s_coreCpuRegisterAF.byte.high);
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
            s_coreCpuRegisterAF.byte.high = coreCpuOpRlBase(s_coreCpuRegisterAF.byte.high);
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
            s_coreCpuRegisterAF.byte.high = coreCpuOpRrBase(s_coreCpuRegisterAF.byte.high);
            s_coreCpuFlagZ = false;
            break;

        case 0x20: // JR NZ, r8
            if(s_coreCpuFlagZ) {
                coreBusCycle();
                s_coreCpuRegisterPC += 1;
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
                s_coreCpuRegisterPC += 1;
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
                s_coreCpuRegisterPC += 1;
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
                s_coreCpuRegisterPC += 1;
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

        case 0x40: // LD B, B
            s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high;
            break;

        case 0x41: // LD B, C
            s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.low;
            break;

        case 0x42: // LD B, D
            s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterDE.byte.high;
            break;

        case 0x43: // LD B, E
            s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterDE.byte.low;
            break;

        case 0x44: // LD B, H
            s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterHL.byte.high;
            break;

        case 0x45: // LD B, L
            s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterHL.byte.low;
            break;

        case 0x46: // LD B, (HL)
            s_coreCpuRegisterBC.byte.high = coreBusRead(s_coreCpuRegisterHL.word);
            break;

        case 0x47: // LD B, A
            s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterAF.byte.high;
            break;

        case 0x48: // LD C, B
            s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.high;
            break;

        case 0x49: // LD C, C
            s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low;
            break;

        case 0x4a: // LD C, D
            s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterDE.byte.high;
            break;

        case 0x4b: // LD C, E
            s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterDE.byte.low;
            break;

        case 0x4c: // LD C, H
            s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterHL.byte.high;
            break;

        case 0x4d: // LD C, L
            s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterHL.byte.low;
            break;

        case 0x4e: // LD C, (HL)
            s_coreCpuRegisterBC.byte.low = coreBusRead(s_coreCpuRegisterHL.word);
            break;

        case 0x4f: // LD C, A
            s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterAF.byte.high;
            break;

        case 0x50: // LD D, B
            s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterBC.byte.high;
            break;

        case 0x51: // LD D, C
            s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterBC.byte.low;
            break;

        case 0x52: // LD D, D
            s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high;
            break;

        case 0x53: // LD D, E
            s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.low;
            break;

        case 0x54: // LD D, H
            s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterHL.byte.high;
            break;

        case 0x55: // LD D, L
            s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterHL.byte.low;
            break;

        case 0x56: // LD D, (HL)
            s_coreCpuRegisterDE.byte.high = coreBusRead(s_coreCpuRegisterHL.word);
            break;

        case 0x57: // LD D, A
            s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterAF.byte.high;
            break;

        case 0x58: // LD E, B
            s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterBC.byte.high;
            break;

        case 0x59: // LD E, C
            s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterBC.byte.low;
            break;

        case 0x5a: // LD E, D
            s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.high;
            break;

        case 0x5b: // LD E, E
            s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low;
            break;

        case 0x5c: // LD E, H
            s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterHL.byte.high;
            break;

        case 0x5d: // LD E, L
            s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterHL.byte.low;
            break;

        case 0x5e: // LD E, (HL)
            s_coreCpuRegisterDE.byte.low = coreBusRead(s_coreCpuRegisterHL.word);
            break;

        case 0x5f: // LD E, A
            s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterAF.byte.high;
            break;

        case 0x60: // LD H, B
            s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterBC.byte.high;
            break;

        case 0x61: // LD H, C
            s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterBC.byte.low;
            break;

        case 0x62: // LD H, D
            s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterDE.byte.high;
            break;

        case 0x63: // LD H, E
            s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterDE.byte.low;
            break;

        case 0x64: // LD H, H
            s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high;
            break;

        case 0x65: // LD H, L
            s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.low;
            break;

        case 0x66: // LD H, (HL)
            s_coreCpuRegisterHL.byte.high = coreBusRead(s_coreCpuRegisterHL.word);
            break;

        case 0x67: // LD H, A
            s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterAF.byte.high;
            break;

        case 0x68: // LD L, B
            s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterBC.byte.high;
            break;

        case 0x69: // LD L, C
            s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterBC.byte.low;
            break;

        case 0x6a: // LD L, D
            s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterDE.byte.high;
            break;

        case 0x6b: // LD L, E
            s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterDE.byte.low;
            break;

        case 0x6c: // LD L, H
            s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.high;
            break;

        case 0x6d: // LD L, L
            s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low;
            break;

        case 0x6e: // LD L, (HL)
            s_coreCpuRegisterHL.byte.low = coreBusRead(s_coreCpuRegisterHL.word);
            break;

        case 0x6f: // LD L, A
            s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterAF.byte.high;
            break;

        case 0x70: // LD (HL), B
            coreBusWrite(s_coreCpuRegisterHL.word, s_coreCpuRegisterBC.byte.high);
            break;

        case 0x71: // LD (HL), C
            coreBusWrite(s_coreCpuRegisterHL.word, s_coreCpuRegisterBC.byte.low);
            break;

        case 0x72: // LD (HL), D
            coreBusWrite(s_coreCpuRegisterHL.word, s_coreCpuRegisterDE.byte.high);
            break;

        case 0x73: // LD (HL), E
            coreBusWrite(s_coreCpuRegisterHL.word, s_coreCpuRegisterDE.byte.low);
            break;

        case 0x74: // LD (HL), H
            coreBusWrite(s_coreCpuRegisterHL.word, s_coreCpuRegisterHL.byte.high);
            break;

        case 0x75: // LD (HL), L
            coreBusWrite(s_coreCpuRegisterHL.word, s_coreCpuRegisterHL.byte.low);
            break;

        case 0x76: // HALT
            // TODO
            break;

        case 0x77: // LD (HL), A
            coreBusWrite(s_coreCpuRegisterHL.word, s_coreCpuRegisterAF.byte.high);
            break;

        case 0x78: // LD A, B
            s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterBC.byte.high;
            break;

        case 0x79: // LD A, C
            s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterBC.byte.low;
            break;

        case 0x7a: // LD A, D
            s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterDE.byte.high;
            break;

        case 0x7b: // LD A, E
            s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterDE.byte.low;
            break;

        case 0x7c: // LD A, H
            s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterHL.byte.high;
            break;

        case 0x7d: // LD A, L
            s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterHL.byte.low;
            break;

        case 0x7e: // LD A, (HL)
            s_coreCpuRegisterAF.byte.high = coreBusRead(s_coreCpuRegisterHL.word);
            break;

        case 0x7f: // LD A, A
            s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high;
            break;

        case 0x80: // ADD A, B
            coreCpuOpAdd(s_coreCpuRegisterBC.byte.high);
            break;

        case 0x81: // ADD A, C
            coreCpuOpAdd(s_coreCpuRegisterBC.byte.low);
            break;

        case 0x82: // ADD A, D
            coreCpuOpAdd(s_coreCpuRegisterDE.byte.high);
            break;

        case 0x83: // ADD A, E
            coreCpuOpAdd(s_coreCpuRegisterDE.byte.low);
            break;

        case 0x84: // ADD A, H
            coreCpuOpAdd(s_coreCpuRegisterHL.byte.high);
            break;

        case 0x85: // ADD A, L
            coreCpuOpAdd(s_coreCpuRegisterHL.byte.low);
            break;

        case 0x86: // ADD A, (HL)
            coreCpuOpAdd(coreBusRead(s_coreCpuRegisterHL.word));
            break;

        case 0x87: // ADD A, A
            coreCpuOpAdd(s_coreCpuRegisterAF.byte.high);
            break;

        case 0x88: // ADC A, B
            coreCpuOpAdc(s_coreCpuRegisterBC.byte.high);
            break;

        case 0x89: // ADC A, C
            coreCpuOpAdc(s_coreCpuRegisterBC.byte.low);
            break;

        case 0x8a: // ADC A, D
            coreCpuOpAdc(s_coreCpuRegisterDE.byte.high);
            break;

        case 0x8b: // ADC A, E
            coreCpuOpAdc(s_coreCpuRegisterDE.byte.low);
            break;

        case 0x8c: // ADC A, H
            coreCpuOpAdc(s_coreCpuRegisterHL.byte.high);
            break;

        case 0x8d: // ADC A, L
            coreCpuOpAdc(s_coreCpuRegisterHL.byte.low);
            break;

        case 0x8e: // ADC A, (HL)
            coreCpuOpAdc(coreBusRead(s_coreCpuRegisterHL.word));
            break;

        case 0x8f: // ADC A, A
            coreCpuOpAdc(s_coreCpuRegisterAF.byte.high);
            break;

        case 0x90: // SUB A, B
            coreCpuOpSub(s_coreCpuRegisterBC.byte.high);
            break;

        case 0x91: // SUB A, C
            coreCpuOpSub(s_coreCpuRegisterBC.byte.low);
            break;

        case 0x92: // SUB A, D
            coreCpuOpSub(s_coreCpuRegisterDE.byte.high);
            break;

        case 0x93: // SUB A, E
            coreCpuOpSub(s_coreCpuRegisterDE.byte.low);
            break;

        case 0x94: // SUB A, H
            coreCpuOpSub(s_coreCpuRegisterHL.byte.high);
            break;

        case 0x95: // SUB A, L
            coreCpuOpSub(s_coreCpuRegisterHL.byte.low);
            break;

        case 0x96: // SUB A, (HL)
            coreCpuOpSub(coreBusRead(s_coreCpuRegisterHL.word));
            break;

        case 0x97: // SUB A, A
            coreCpuOpSub(s_coreCpuRegisterAF.byte.high);
            break;

        case 0x98: // SBC A, B
            coreCpuOpSbc(s_coreCpuRegisterBC.byte.high);
            break;

        case 0x99: // SBC A, C
            coreCpuOpSbc(s_coreCpuRegisterBC.byte.low);
            break;

        case 0x9a: // SBC A, D
            coreCpuOpSbc(s_coreCpuRegisterDE.byte.high);
            break;

        case 0x9b: // SBC A, E
            coreCpuOpSbc(s_coreCpuRegisterDE.byte.low);
            break;

        case 0x9c: // SBC A, H
            coreCpuOpSbc(s_coreCpuRegisterHL.byte.high);
            break;

        case 0x9d: // SBC A, L
            coreCpuOpSbc(s_coreCpuRegisterHL.byte.low);
            break;

        case 0x9e: // SBC A, (HL)
            coreCpuOpSbc(coreBusRead(s_coreCpuRegisterHL.word));
            break;

        case 0x9f: // SBC A, A
            coreCpuOpSbc(s_coreCpuRegisterAF.byte.high);
            break;

        case 0xa0: // AND A, B
            coreCpuOpAnd(s_coreCpuRegisterBC.byte.high);
            break;

        case 0xa1: // AND A, C
            coreCpuOpAnd(s_coreCpuRegisterBC.byte.low);
            break;

        case 0xa2: // AND A, D
            coreCpuOpAnd(s_coreCpuRegisterDE.byte.high);
            break;

        case 0xa3: // AND A, E
            coreCpuOpAnd(s_coreCpuRegisterDE.byte.low);
            break;

        case 0xa4: // AND A, H
            coreCpuOpAnd(s_coreCpuRegisterHL.byte.high);
            break;

        case 0xa5: // AND A, L
            coreCpuOpAnd(s_coreCpuRegisterHL.byte.low);
            break;

        case 0xa6: // AND A, (HL)
            coreCpuOpAnd(coreBusRead(s_coreCpuRegisterHL.word));
            break;

        case 0xa7: // AND A, A
            coreCpuOpAnd(s_coreCpuRegisterAF.byte.high);
            break;

        case 0xa8: // XOR A, B
            coreCpuOpXor(s_coreCpuRegisterBC.byte.high);
            break;

        case 0xa9: // XOR A, C
            coreCpuOpXor(s_coreCpuRegisterBC.byte.low);
            break;

        case 0xaa: // XOR A, D
            coreCpuOpXor(s_coreCpuRegisterDE.byte.high);
            break;

        case 0xab: // XOR A, E
            coreCpuOpXor(s_coreCpuRegisterDE.byte.low);
            break;

        case 0xac: // XOR A, H
            coreCpuOpXor(s_coreCpuRegisterHL.byte.high);
            break;

        case 0xad: // XOR A, L
            coreCpuOpXor(s_coreCpuRegisterHL.byte.low);
            break;

        case 0xae: // XOR A, (HL)
            coreCpuOpXor(coreBusRead(s_coreCpuRegisterHL.word));
            break;

        case 0xaf: // XOR A, A
            coreCpuOpXor(s_coreCpuRegisterAF.byte.high);
            break;

        case 0xb0: // OR A, B
            coreCpuOpOr(s_coreCpuRegisterBC.byte.high);
            break;

        case 0xb1: // OR A, C
            coreCpuOpOr(s_coreCpuRegisterBC.byte.low);
            break;

        case 0xb2: // OR A, D
            coreCpuOpOr(s_coreCpuRegisterDE.byte.high);
            break;

        case 0xb3: // OR A, E
            coreCpuOpOr(s_coreCpuRegisterDE.byte.low);
            break;

        case 0xb4: // OR A, H
            coreCpuOpOr(s_coreCpuRegisterHL.byte.high);
            break;

        case 0xb5: // OR A, L
            coreCpuOpOr(s_coreCpuRegisterHL.byte.low);
            break;

        case 0xb6: // OR A, (HL)
            coreCpuOpOr(coreBusRead(s_coreCpuRegisterHL.word));
            break;

        case 0xb7: // OR A, A
            coreCpuOpOr(s_coreCpuRegisterAF.byte.high);
            break;

        case 0xb8: // CP A, B
            coreCpuOpCp(s_coreCpuRegisterBC.byte.high);
            break;

        case 0xb9: // CP A, C
            coreCpuOpCp(s_coreCpuRegisterBC.byte.low);
            break;

        case 0xba: // CP A, D
            coreCpuOpCp(s_coreCpuRegisterDE.byte.high);
            break;

        case 0xbb: // CP A, E
            coreCpuOpCp(s_coreCpuRegisterDE.byte.low);
            break;

        case 0xbc: // CP A, H
            coreCpuOpCp(s_coreCpuRegisterHL.byte.high);
            break;

        case 0xbd: // CP A, L
            coreCpuOpCp(s_coreCpuRegisterHL.byte.low);
            break;

        case 0xbe: // CP A, (HL)
            coreCpuOpCp(coreBusRead(s_coreCpuRegisterHL.word));
            break;

        case 0xbf: // CP A, A
            coreCpuOpCp(s_coreCpuRegisterAF.byte.high);
            break;

        case 0xc0: // RET NZ
            coreBusCycle();

            if(!s_coreCpuFlagZ) {
                s_coreCpuRegisterPC = coreCpuPop();
                coreBusCycle();
            }

            break;

        case 0xc1: // POP BC
            s_coreCpuRegisterBC.word = coreCpuPop();
            break;

        case 0xc2: // JP NZ, a16
            if(s_coreCpuFlagZ) {
                coreBusCycle();
                s_coreCpuRegisterPC += 2;
            } else {
                s_coreCpuRegisterPC = coreCpuFetch16();
            }

            coreBusCycle();

            break;

        case 0xc3: // JP a16
            s_coreCpuRegisterPC = coreCpuFetch16();
            coreBusCycle();
            break;

        case 0xc4: // CALL NZ, a16
            if(s_coreCpuFlagZ) {
                coreBusCycle();
                coreBusCycle();
                s_coreCpuRegisterPC += 2;
            } else {
                uint16_t l_operand = coreCpuFetch16();
                coreBusCycle();
                coreCpuPush(s_coreCpuRegisterPC);
                s_coreCpuRegisterPC = l_operand;
            }

            break;

        case 0xc5: // PUSH BC
            coreBusCycle();
            coreCpuPush(s_coreCpuRegisterBC.word);
            break;

        case 0xc6: // ADD a, d8
            coreCpuOpAdd(coreCpuFetch8());
            break;

        case 0xc7: // RST 0x00
            coreBusCycle();
            coreCpuPush(s_coreCpuRegisterPC);
            s_coreCpuRegisterPC = 0x00;
            break;

        case 0xc8: // RET Z
            coreBusCycle();

            if(s_coreCpuFlagZ) {
                s_coreCpuRegisterPC = coreCpuPop();
                coreBusCycle();
            }

            break;

        case 0xc9: // RET
            s_coreCpuRegisterPC = coreCpuPop();
            coreBusCycle();
            break;

        case 0xca: // JP Z, a16
            if(s_coreCpuFlagZ) {
                s_coreCpuRegisterPC = coreCpuFetch16();
            } else {
                coreBusCycle();
                s_coreCpuRegisterPC += 2;
            }

            coreBusCycle();

            break;

        case 0xcb: // Prefix
            switch(coreCpuFetch8()) {
                case 0x00: // RLC B
                    s_coreCpuRegisterBC.byte.high = coreCpuOpRlc(s_coreCpuRegisterBC.byte.high);
                    break;

                case 0x01: // RLC C
                    s_coreCpuRegisterBC.byte.low = coreCpuOpRlc(s_coreCpuRegisterBC.byte.low);
                    break;

                case 0x02: // RLC D
                    s_coreCpuRegisterDE.byte.high = coreCpuOpRlc(s_coreCpuRegisterDE.byte.high);
                    break;

                case 0x03: // RLC E
                    s_coreCpuRegisterDE.byte.low = coreCpuOpRlc(s_coreCpuRegisterDE.byte.low);
                    break;

                case 0x04: // RLC H
                    s_coreCpuRegisterHL.byte.high = coreCpuOpRlc(s_coreCpuRegisterHL.byte.high);
                    break;

                case 0x05: // RLC L
                    s_coreCpuRegisterHL.byte.low = coreCpuOpRlc(s_coreCpuRegisterHL.byte.low);
                    break;

                case 0x06: // RLC (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreCpuOpRlc(coreBusRead(s_coreCpuRegisterHL.word)));
                    break;

                case 0x07: // RLC A
                    s_coreCpuRegisterAF.byte.high = coreCpuOpRlc(s_coreCpuRegisterAF.byte.high);
                    break;

                case 0x08: // RRC B
                    s_coreCpuRegisterBC.byte.high = coreCpuOpRrc(s_coreCpuRegisterBC.byte.high);
                    break;

                case 0x09: // RRC C
                    s_coreCpuRegisterBC.byte.low = coreCpuOpRrc(s_coreCpuRegisterBC.byte.low);
                    break;

                case 0x0a: // RRC D
                    s_coreCpuRegisterDE.byte.high = coreCpuOpRrc(s_coreCpuRegisterDE.byte.high);
                    break;

                case 0x0b: // RRC E
                    s_coreCpuRegisterDE.byte.low = coreCpuOpRrc(s_coreCpuRegisterDE.byte.low);
                    break;

                case 0x0c: // RRC H
                    s_coreCpuRegisterHL.byte.high = coreCpuOpRrc(s_coreCpuRegisterHL.byte.high);
                    break;

                case 0x0d: // RRC L
                    s_coreCpuRegisterHL.byte.low = coreCpuOpRrc(s_coreCpuRegisterHL.byte.low);
                    break;

                case 0x0e: // RRC (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreCpuOpRrc(coreBusRead(s_coreCpuRegisterHL.word)));
                    break;

                case 0x0f: // RRC A
                    s_coreCpuRegisterAF.byte.high = coreCpuOpRrc(s_coreCpuRegisterAF.byte.high);
                    break;

                case 0x10: // RL B
                    s_coreCpuRegisterBC.byte.high = coreCpuOpRl(s_coreCpuRegisterBC.byte.high);
                    break;

                case 0x11: // RL C
                    s_coreCpuRegisterBC.byte.low = coreCpuOpRl(s_coreCpuRegisterBC.byte.low);
                    break;

                case 0x12: // RL D
                    s_coreCpuRegisterDE.byte.high = coreCpuOpRl(s_coreCpuRegisterDE.byte.high);
                    break;

                case 0x13: // RL E
                    s_coreCpuRegisterDE.byte.low = coreCpuOpRl(s_coreCpuRegisterDE.byte.low);
                    break;

                case 0x14: // RL H
                    s_coreCpuRegisterHL.byte.high = coreCpuOpRl(s_coreCpuRegisterHL.byte.high);
                    break;

                case 0x15: // RL L
                    s_coreCpuRegisterHL.byte.low = coreCpuOpRl(s_coreCpuRegisterHL.byte.low);
                    break;

                case 0x16: // RL (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreCpuOpRl(coreBusRead(s_coreCpuRegisterHL.word)));
                    break;

                case 0x17: // RL A
                    s_coreCpuRegisterAF.byte.high = coreCpuOpRl(s_coreCpuRegisterAF.byte.high);
                    break;

                case 0x18: // RR B
                    s_coreCpuRegisterBC.byte.high = coreCpuOpRr(s_coreCpuRegisterBC.byte.high);
                    break;

                case 0x19: // RR C
                    s_coreCpuRegisterBC.byte.low = coreCpuOpRr(s_coreCpuRegisterBC.byte.low);
                    break;

                case 0x1a: // RR D
                    s_coreCpuRegisterDE.byte.high = coreCpuOpRr(s_coreCpuRegisterDE.byte.high);
                    break;

                case 0x1b: // RR E
                    s_coreCpuRegisterDE.byte.low = coreCpuOpRr(s_coreCpuRegisterDE.byte.low);
                    break;

                case 0x1c: // RR H
                    s_coreCpuRegisterHL.byte.high = coreCpuOpRr(s_coreCpuRegisterHL.byte.high);
                    break;

                case 0x1d: // RR L
                    s_coreCpuRegisterHL.byte.low = coreCpuOpRr(s_coreCpuRegisterHL.byte.low);
                    break;

                case 0x1e: // RR (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreCpuOpRr(coreBusRead(s_coreCpuRegisterHL.word)));
                    break;

                case 0x1f: // RR A
                    s_coreCpuRegisterAF.byte.high = coreCpuOpRr(s_coreCpuRegisterAF.byte.high);
                    break;

                case 0x20: // SLA B
                    s_coreCpuRegisterBC.byte.high = coreCpuOpSla(s_coreCpuRegisterBC.byte.high);
                    break;

                case 0x21: // SLA C
                    s_coreCpuRegisterBC.byte.low = coreCpuOpSla(s_coreCpuRegisterBC.byte.low);
                    break;

                case 0x22: // SLA D
                    s_coreCpuRegisterDE.byte.high = coreCpuOpSla(s_coreCpuRegisterDE.byte.high);
                    break;

                case 0x23: // SLA E
                    s_coreCpuRegisterDE.byte.low = coreCpuOpSla(s_coreCpuRegisterDE.byte.low);
                    break;

                case 0x24: // SLA H
                    s_coreCpuRegisterHL.byte.high = coreCpuOpSla(s_coreCpuRegisterHL.byte.high);
                    break;

                case 0x25: // SLA L
                    s_coreCpuRegisterHL.byte.low = coreCpuOpSla(s_coreCpuRegisterHL.byte.low);
                    break;

                case 0x26: // SLA (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreCpuOpSla(coreBusRead(s_coreCpuRegisterHL.word)));
                    break;

                case 0x27: // SLA A
                    s_coreCpuRegisterAF.byte.high = coreCpuOpSla(s_coreCpuRegisterAF.byte.high);
                    break;

                case 0x28: // SRA B
                    s_coreCpuRegisterBC.byte.high = coreCpuOpSra(s_coreCpuRegisterBC.byte.high);
                    break;

                case 0x29: // SRA C
                    s_coreCpuRegisterBC.byte.low = coreCpuOpSra(s_coreCpuRegisterBC.byte.low);
                    break;

                case 0x2a: // SRA D
                    s_coreCpuRegisterDE.byte.high = coreCpuOpSra(s_coreCpuRegisterDE.byte.high);
                    break;

                case 0x2b: // SRA E
                    s_coreCpuRegisterDE.byte.low = coreCpuOpSra(s_coreCpuRegisterDE.byte.low);
                    break;

                case 0x2c: // SRA H
                    s_coreCpuRegisterHL.byte.high = coreCpuOpSra(s_coreCpuRegisterHL.byte.high);
                    break;

                case 0x2d: // SRA L
                    s_coreCpuRegisterHL.byte.low = coreCpuOpSra(s_coreCpuRegisterHL.byte.low);
                    break;

                case 0x2e: // SRA (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreCpuOpSra(coreBusRead(s_coreCpuRegisterHL.word)));
                    break;

                case 0x2f: // SRA A
                    s_coreCpuRegisterAF.byte.high = coreCpuOpSra(s_coreCpuRegisterAF.byte.high);
                    break;

                case 0x30: // SWAP B
                    s_coreCpuRegisterBC.byte.high = coreCpuOpSwap(s_coreCpuRegisterBC.byte.high);
                    break;

                case 0x31: // SWAP C
                    s_coreCpuRegisterBC.byte.low = coreCpuOpSwap(s_coreCpuRegisterBC.byte.low);
                    break;

                case 0x32: // SWAP D
                    s_coreCpuRegisterDE.byte.high = coreCpuOpSwap(s_coreCpuRegisterDE.byte.high);
                    break;

                case 0x33: // SWAP E
                    s_coreCpuRegisterDE.byte.low = coreCpuOpSwap(s_coreCpuRegisterDE.byte.low);
                    break;

                case 0x34: // SWAP H
                    s_coreCpuRegisterHL.byte.high = coreCpuOpSwap(s_coreCpuRegisterHL.byte.high);
                    break;

                case 0x35: // SWAP L
                    s_coreCpuRegisterHL.byte.low = coreCpuOpSwap(s_coreCpuRegisterHL.byte.low);
                    break;

                case 0x36: // SWAP (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreCpuOpSwap(coreBusRead(s_coreCpuRegisterHL.word)));
                    break;

                case 0x37: // SWAP A
                    s_coreCpuRegisterAF.byte.high = coreCpuOpSwap(s_coreCpuRegisterAF.byte.high);
                    break;

                case 0x38: // SRL B
                    s_coreCpuRegisterBC.byte.high = coreCpuOpSrl(s_coreCpuRegisterBC.byte.high);
                    break;

                case 0x39: // SRL C
                    s_coreCpuRegisterBC.byte.low = coreCpuOpSrl(s_coreCpuRegisterBC.byte.low);
                    break;

                case 0x3a: // SRL D
                    s_coreCpuRegisterDE.byte.high = coreCpuOpSrl(s_coreCpuRegisterDE.byte.high);
                    break;

                case 0x3b: // SRL E
                    s_coreCpuRegisterDE.byte.low = coreCpuOpSrl(s_coreCpuRegisterDE.byte.low);
                    break;

                case 0x3c: // SRL H
                    s_coreCpuRegisterHL.byte.high = coreCpuOpSrl(s_coreCpuRegisterHL.byte.high);
                    break;

                case 0x3d: // SRL L
                    s_coreCpuRegisterHL.byte.low = coreCpuOpSrl(s_coreCpuRegisterHL.byte.low);
                    break;

                case 0x3e: // SRL (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreCpuOpSrl(coreBusRead(s_coreCpuRegisterHL.word)));
                    break;

                case 0x3f: // SRL A
                    s_coreCpuRegisterAF.byte.high = coreCpuOpSrl(s_coreCpuRegisterAF.byte.high);
                    break;

                case 0x40: // BIT 0, B
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.high & (1 << 0)) == 0;
                    break;

                case 0x41: // BIT 0, C
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.low & (1 << 0)) == 0;
                    break;

                case 0x42: // BIT 0, D
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.high & (1 << 0)) == 0;
                    break;

                case 0x43: // BIT 0, E
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.low & (1 << 0)) == 0;
                    break;

                case 0x44: // BIT 0, H
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.high & (1 << 0)) == 0;
                    break;

                case 0x45: // BIT 0, L
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.low & (1 << 0)) == 0;
                    break;

                case 0x46: // BIT 0, (HL)
                    s_coreCpuFlagZ = (coreBusRead(s_coreCpuRegisterHL.word) & (1 << 0)) == 0;
                    break;

                case 0x47: // BIT 0, A
                    s_coreCpuFlagZ = (s_coreCpuRegisterAF.byte.high & (1 << 0)) == 0;
                    break;

                case 0x48: // BIT 1, B
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.high & (1 << 1)) == 0;
                    break;

                case 0x49: // BIT 1, C
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.low & (1 << 1)) == 0;
                    break;

                case 0x4a: // BIT 1, D
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.high & (1 << 1)) == 0;
                    break;

                case 0x4b: // BIT 1, E
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.low & (1 << 1)) == 0;
                    break;

                case 0x4c: // BIT 1, H
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.high & (1 << 1)) == 0;
                    break;

                case 0x4d: // BIT 1, L
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.low & (1 << 1)) == 0;
                    break;

                case 0x4e: // BIT 1, (HL)
                    s_coreCpuFlagZ = (coreBusRead(s_coreCpuRegisterHL.word) & (1 << 1)) == 0;
                    break;

                case 0x4f: // BIT 1, A
                    s_coreCpuFlagZ = (s_coreCpuRegisterAF.byte.high & (1 << 1)) == 0;
                    break;

                case 0x50: // BIT 2, B
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.high & (1 << 2)) == 0;
                    break;

                case 0x51: // BIT 2, C
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.low & (1 << 2)) == 0;
                    break;

                case 0x52: // BIT 2, D
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.high & (1 << 2)) == 0;
                    break;

                case 0x53: // BIT 2, E
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.low & (1 << 2)) == 0;
                    break;

                case 0x54: // BIT 2, H
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.high & (1 << 2)) == 0;
                    break;

                case 0x55: // BIT 2, L
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.low & (1 << 2)) == 0;
                    break;

                case 0x56: // BIT 2, (HL)
                    s_coreCpuFlagZ = (coreBusRead(s_coreCpuRegisterHL.word) & (1 << 2)) == 0;
                    break;

                case 0x57: // BIT 2, A
                    s_coreCpuFlagZ = (s_coreCpuRegisterAF.byte.high & (1 << 2)) == 0;
                    break;

                case 0x58: // BIT 3, B
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.high & (1 << 3)) == 0;
                    break;

                case 0x59: // BIT 3, C
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.low & (1 << 3)) == 0;
                    break;

                case 0x5a: // BIT 3, D
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.high & (1 << 3)) == 0;
                    break;

                case 0x5b: // BIT 3, E
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.low & (1 << 3)) == 0;
                    break;

                case 0x5c: // BIT 3, H
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.high & (1 << 3)) == 0;
                    break;

                case 0x5d: // BIT 3, L
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.low & (1 << 3)) == 0;
                    break;

                case 0x5e: // BIT 3, (HL)
                    s_coreCpuFlagZ = (coreBusRead(s_coreCpuRegisterHL.word) & (1 << 3)) == 0;
                    break;

                case 0x5f: // BIT 3, A
                    s_coreCpuFlagZ = (s_coreCpuRegisterAF.byte.high & (1 << 3)) == 0;
                    break;

                case 0x60: // BIT 4, B
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.high & (1 << 4)) == 0;
                    break;

                case 0x61: // BIT 4, C
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.low & (1 << 4)) == 0;
                    break;

                case 0x62: // BIT 4, D
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.high & (1 << 4)) == 0;
                    break;

                case 0x63: // BIT 4, E
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.low & (1 << 4)) == 0;
                    break;

                case 0x64: // BIT 4, H
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.high & (1 << 4)) == 0;
                    break;

                case 0x65: // BIT 4, L
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.low & (1 << 4)) == 0;
                    break;

                case 0x66: // BIT 4, (HL)
                    s_coreCpuFlagZ = (coreBusRead(s_coreCpuRegisterHL.word) & (1 << 4)) == 0;
                    break;

                case 0x67: // BIT 4, A
                    s_coreCpuFlagZ = (s_coreCpuRegisterAF.byte.high & (1 << 4)) == 0;
                    break;

                case 0x68: // BIT 5, B
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.high & (1 << 5)) == 0;
                    break;

                case 0x69: // BIT 5, C
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.low & (1 << 5)) == 0;
                    break;

                case 0x6a: // BIT 5, D
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.high & (1 << 5)) == 0;
                    break;

                case 0x6b: // BIT 5, E
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.low & (1 << 5)) == 0;
                    break;

                case 0x6c: // BIT 5, H
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.high & (1 << 5)) == 0;
                    break;

                case 0x6d: // BIT 5, L
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.low & (1 << 5)) == 0;
                    break;

                case 0x6e: // BIT 5, (HL)
                    s_coreCpuFlagZ = (coreBusRead(s_coreCpuRegisterHL.word) & (1 << 5)) == 0;
                    break;

                case 0x6f: // BIT 5, A
                    s_coreCpuFlagZ = (s_coreCpuRegisterAF.byte.high & (1 << 5)) == 0;
                    break;

                case 0x70: // BIT 6, B
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.high & (1 << 6)) == 0;
                    break;

                case 0x71: // BIT 6, C
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.low & (1 << 6)) == 0;
                    break;

                case 0x72: // BIT 6, D
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.high & (1 << 6)) == 0;
                    break;

                case 0x73: // BIT 6, E
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.low & (1 << 6)) == 0;
                    break;

                case 0x74: // BIT 6, H
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.high & (1 << 6)) == 0;
                    break;

                case 0x75: // BIT 6, L
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.low & (1 << 6)) == 0;
                    break;

                case 0x76: // BIT 6, (HL)
                    s_coreCpuFlagZ = (coreBusRead(s_coreCpuRegisterHL.word) & (1 << 6)) == 0;
                    break;

                case 0x77: // BIT 6, A
                    s_coreCpuFlagZ = (s_coreCpuRegisterAF.byte.high & (1 << 6)) == 0;
                    break;

                case 0x78: // BIT 7, B
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.high & (1 << 7)) == 0;
                    break;

                case 0x79: // BIT 7, C
                    s_coreCpuFlagZ = (s_coreCpuRegisterBC.byte.low & (1 << 7)) == 0;
                    break;

                case 0x7a: // BIT 7, D
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.high & (1 << 7)) == 0;
                    break;

                case 0x7b: // BIT 7, E
                    s_coreCpuFlagZ = (s_coreCpuRegisterDE.byte.low & (1 << 7)) == 0;
                    break;

                case 0x7c: // BIT 7, H
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.high & (1 << 7)) == 0;
                    break;

                case 0x7d: // BIT 7, L
                    s_coreCpuFlagZ = (s_coreCpuRegisterHL.byte.low & (1 << 7)) == 0;
                    break;

                case 0x7e: // BIT 7, (HL)
                    s_coreCpuFlagZ = (coreBusRead(s_coreCpuRegisterHL.word) & (1 << 7)) == 0;
                    break;

                case 0x7f: // BIT 7, A
                    s_coreCpuFlagZ = (s_coreCpuRegisterAF.byte.high & (1 << 7)) == 0;
                    break;

                case 0x80: // RES 0, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high & ~(1 << 0);
                    break;

                case 0x81: // RES 0, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low & ~(1 << 0);
                    break;

                case 0x82: // RES 0, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high & ~(1 << 0);
                    break;

                case 0x83: // RES 0, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low & ~(1 << 0);
                    break;

                case 0x84: // RES 0, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high & ~(1 << 0);
                    break;

                case 0x85: // RES 0, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low & ~(1 << 0);
                    break;

                case 0x86: // RES 0, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) & ~(1 << 0));
                    break;

                case 0x87: // RES 0, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high & ~(1 << 0);
                    break;

                case 0x88: // RES 1, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high & ~(1 << 1);
                    break;

                case 0x89: // RES 1, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low & ~(1 << 1);
                    break;

                case 0x8a: // RES 1, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high & ~(1 << 1);
                    break;

                case 0x8b: // RES 1, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low & ~(1 << 1);
                    break;

                case 0x8c: // RES 1, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high & ~(1 << 1);
                    break;

                case 0x8d: // RES 1, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low & ~(1 << 1);
                    break;

                case 0x8e: // RES 1, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) & ~(1 << 1));
                    break;

                case 0x8f: // RES 1, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high & ~(1 << 1);
                    break;

                case 0x90: // RES 2, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high & ~(1 << 2);
                    break;

                case 0x91: // RES 2, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low & ~(1 << 2);
                    break;

                case 0x92: // RES 2, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high & ~(1 << 2);
                    break;

                case 0x93: // RES 2, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low & ~(1 << 2);
                    break;

                case 0x94: // RES 2, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high & ~(1 << 2);
                    break;

                case 0x95: // RES 2, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low & ~(1 << 2);
                    break;

                case 0x96: // RES 2, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) & ~(1 << 2));
                    break;

                case 0x97: // RES 2, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high & ~(1 << 2);
                    break;

                case 0x98: // RES 3, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high & ~(1 << 3);
                    break;

                case 0x99: // RES 3, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low & ~(1 << 3);
                    break;

                case 0x9a: // RES 3, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high & ~(1 << 3);
                    break;

                case 0x9b: // RES 3, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low & ~(1 << 3);
                    break;

                case 0x9c: // RES 3, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high & ~(1 << 3);
                    break;

                case 0x9d: // RES 3, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low & ~(1 << 3);
                    break;

                case 0x9e: // RES 3, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) & ~(1 << 3));
                    break;

                case 0x9f: // RES 3, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high & ~(1 << 3);
                    break;

                case 0xa0: // RES 4, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high & ~(1 << 4);
                    break;

                case 0xa1: // RES 4, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low & ~(1 << 4);
                    break;

                case 0xa2: // RES 4, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high & ~(1 << 4);
                    break;

                case 0xa3: // RES 4, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low & ~(1 << 4);
                    break;

                case 0xa4: // RES 4, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high & ~(1 << 4);
                    break;

                case 0xa5: // RES 4, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low & ~(1 << 4);
                    break;

                case 0xa6: // RES 4, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) & ~(1 << 4));
                    break;

                case 0xa7: // RES 4, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high & ~(1 << 4);
                    break;

                case 0xa8: // RES 5, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high & ~(1 << 5);
                    break;

                case 0xa9: // RES 5, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low & ~(1 << 5);
                    break;

                case 0xaa: // RES 5, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high & ~(1 << 5);
                    break;

                case 0xab: // RES 5, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low & ~(1 << 5);
                    break;

                case 0xac: // RES 5, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high & ~(1 << 5);
                    break;

                case 0xad: // RES 5, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low & ~(1 << 5);
                    break;

                case 0xae: // RES 5, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) & ~(1 << 5));
                    break;

                case 0xaf: // RES 5, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high & ~(1 << 5);
                    break;

                case 0xb0: // RES 6, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high & ~(1 << 6);
                    break;

                case 0xb1: // RES 6, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low & ~(1 << 6);
                    break;

                case 0xb2: // RES 6, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high & ~(1 << 6);
                    break;

                case 0xb3: // RES 6, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low & ~(1 << 6);
                    break;

                case 0xb4: // RES 6, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high & ~(1 << 6);
                    break;

                case 0xb5: // RES 6, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low & ~(1 << 6);
                    break;

                case 0xb6: // RES 6, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) & ~(1 << 6));
                    break;

                case 0xb7: // RES 6, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high & ~(1 << 6);
                    break;

                case 0xb8: // RES 7, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high & ~(1 << 7);
                    break;

                case 0xb9: // RES 7, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low & ~(1 << 7);
                    break;

                case 0xba: // RES 7, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high & ~(1 << 7);
                    break;

                case 0xbb: // RES 7, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low & ~(1 << 7);
                    break;

                case 0xbc: // RES 7, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high & ~(1 << 7);
                    break;

                case 0xbd: // RES 7, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low & ~(1 << 7);
                    break;

                case 0xbe: // RES 7, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) & ~(1 << 7));
                    break;

                case 0xbf: // RES 7, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high & ~(1 << 7);
                    break;

                case 0xc0: // SET 0, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high | (1 << 0);
                    break;

                case 0xc1: // SET 0, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low | (1 << 0);
                    break;

                case 0xc2: // SET 0, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high | (1 << 0);
                    break;

                case 0xc3: // SET 0, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low | (1 << 0);
                    break;

                case 0xc4: // SET 0, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high | (1 << 0);
                    break;

                case 0xc5: // SET 0, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low | (1 << 0);
                    break;

                case 0xc6: // SET 0, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) | (1 << 0));
                    break;

                case 0xc7: // SET 0, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high | (1 << 0);
                    break;

                case 0xc8: // SET 1, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high | (1 << 1);
                    break;

                case 0xc9: // SET 1, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low | (1 << 1);
                    break;

                case 0xca: // SET 1, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high | (1 << 1);
                    break;

                case 0xcb: // SET 1, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low | (1 << 1);
                    break;

                case 0xcc: // SET 1, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high | (1 << 1);
                    break;

                case 0xcd: // SET 1, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low | (1 << 1);
                    break;

                case 0xce: // SET 1, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) | (1 << 1));
                    break;

                case 0xcf: // SET 1, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high | (1 << 1);
                    break;

                case 0xd0: // SET 2, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high | (1 << 2);
                    break;

                case 0xd1: // SET 2, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low | (1 << 2);
                    break;

                case 0xd2: // SET 2, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high | (1 << 2);
                    break;

                case 0xd3: // SET 2, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low | (1 << 2);
                    break;

                case 0xd4: // SET 2, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high | (1 << 2);
                    break;

                case 0xd5: // SET 2, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low | (1 << 2);
                    break;

                case 0xd6: // SET 2, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) | (1 << 2));
                    break;

                case 0xd7: // SET 2, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high | (1 << 2);
                    break;

                case 0xd8: // SET 3, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high | (1 << 3);
                    break;

                case 0xd9: // SET 3, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low | (1 << 3);
                    break;

                case 0xda: // SET 3, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high | (1 << 3);
                    break;

                case 0xdb: // SET 3, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low | (1 << 3);
                    break;

                case 0xdc: // SET 3, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high | (1 << 3);
                    break;

                case 0xdd: // SET 3, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low | (1 << 3);
                    break;

                case 0xde: // SET 3, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) | (1 << 3));
                    break;

                case 0xdf: // SET 3, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high | (1 << 3);
                    break;

                case 0xe0: // SET 4, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high | (1 << 4);
                    break;

                case 0xe1: // SET 4, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low | (1 << 4);
                    break;

                case 0xe2: // SET 4, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high | (1 << 4);
                    break;

                case 0xe3: // SET 4, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low | (1 << 4);
                    break;

                case 0xe4: // SET 4, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high | (1 << 4);
                    break;

                case 0xe5: // SET 4, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low | (1 << 4);
                    break;

                case 0xe6: // SET 4, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) | (1 << 4));
                    break;

                case 0xe7: // SET 4, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high | (1 << 4);
                    break;

                case 0xe8: // SET 5, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high | (1 << 5);
                    break;

                case 0xe9: // SET 5, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low | (1 << 5);
                    break;

                case 0xea: // SET 5, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high | (1 << 5);
                    break;

                case 0xeb: // SET 5, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low | (1 << 5);
                    break;

                case 0xec: // SET 5, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high | (1 << 5);
                    break;

                case 0xed: // SET 5, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low | (1 << 5);
                    break;

                case 0xee: // SET 5, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) | (1 << 5));
                    break;

                case 0xef: // SET 5, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high | (1 << 5);
                    break;

                case 0xf0: // SET 6, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high | (1 << 6);
                    break;

                case 0xf1: // SET 6, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low | (1 << 6);
                    break;

                case 0xf2: // SET 6, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high | (1 << 6);
                    break;

                case 0xf3: // SET 6, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low | (1 << 6);
                    break;

                case 0xf4: // SET 6, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high | (1 << 6);
                    break;

                case 0xf5: // SET 6, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low | (1 << 6);
                    break;

                case 0xf6: // SET 6, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) | (1 << 6));
                    break;

                case 0xf7: // SET 6, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high | (1 << 6);
                    break;

                case 0xf8: // SET 7, B
                    s_coreCpuRegisterBC.byte.high = s_coreCpuRegisterBC.byte.high | (1 << 7);
                    break;

                case 0xf9: // SET 7, C
                    s_coreCpuRegisterBC.byte.low = s_coreCpuRegisterBC.byte.low | (1 << 7);
                    break;

                case 0xfa: // SET 7, D
                    s_coreCpuRegisterDE.byte.high = s_coreCpuRegisterDE.byte.high | (1 << 7);
                    break;

                case 0xfb: // SET 7, E
                    s_coreCpuRegisterDE.byte.low = s_coreCpuRegisterDE.byte.low | (1 << 7);
                    break;

                case 0xfc: // SET 7, H
                    s_coreCpuRegisterHL.byte.high = s_coreCpuRegisterHL.byte.high | (1 << 7);
                    break;

                case 0xfd: // SET 7, L
                    s_coreCpuRegisterHL.byte.low = s_coreCpuRegisterHL.byte.low | (1 << 7);
                    break;

                case 0xfe: // SET 7, (HL)
                    coreBusWrite(s_coreCpuRegisterHL.word, coreBusRead(s_coreCpuRegisterHL.word) | (1 << 7));
                    break;

                case 0xff: // SET 7, A
                    s_coreCpuRegisterAF.byte.high = s_coreCpuRegisterAF.byte.high | (1 << 7);
                    break;
            }

            break;

        case 0xcc: // CALL Z, a16
            if(s_coreCpuFlagZ) {
                uint16_t l_operand = coreCpuFetch16();
                coreBusCycle();
                coreCpuPush(s_coreCpuRegisterPC);
                s_coreCpuRegisterPC = l_operand;
            } else {
                coreBusCycle();
                coreBusCycle();
                s_coreCpuRegisterPC += 2;
            }

            break;

        case 0xcd: // CALL a16
            {
                uint16_t l_address = coreCpuFetch16();
                coreBusCycle();
                coreCpuPush(s_coreCpuRegisterPC);
                s_coreCpuRegisterPC = l_address;
            }

            break;

        case 0xce: // ADC A, d8
            coreCpuOpAdc(coreCpuFetch8());
            break;

        case 0xcf: // RST 0x08
            coreBusCycle();
            coreCpuPush(s_coreCpuRegisterPC);
            s_coreCpuRegisterPC = 0x08;
            break;

        case 0xd0: // RET NC
            coreBusCycle();

            if(!s_coreCpuFlagC) {
                s_coreCpuRegisterPC = coreCpuPop();
                coreBusCycle();
            }

            break;

        case 0xd1: // POP DE
            s_coreCpuRegisterDE.word = coreCpuPop();
            break;

        case 0xd2: // JP NC, a16
            if(s_coreCpuFlagC) {
                coreBusCycle();
                s_coreCpuRegisterPC += 2;
            } else {
                s_coreCpuRegisterPC = coreCpuFetch16();
            }

            coreBusCycle();

            break;

        case 0xd4: // CALL NC, a16
            if(s_coreCpuFlagC) {
                coreBusCycle();
                coreBusCycle();
                s_coreCpuRegisterPC += 2;
            } else {
                uint16_t l_operand = coreCpuFetch16();
                coreBusCycle();
                coreCpuPush(s_coreCpuRegisterPC);
                s_coreCpuRegisterPC = l_operand;
            }

            break;

        case 0xd5: // PUSH DE
            coreBusCycle();
            coreCpuPush(s_coreCpuRegisterDE.word);
            break;

        case 0xd6: // SUB A, d8
            coreCpuOpSub(coreCpuFetch8());
            break;

        case 0xd7: // RST 0x10
            coreBusCycle();
            coreCpuPush(s_coreCpuRegisterPC);
            s_coreCpuRegisterPC = 0x10;
            break;

        case 0xd8: // RET C
            coreBusCycle();

            if(s_coreCpuFlagC) {
                s_coreCpuRegisterPC = coreCpuPop();
                coreBusCycle();
            }

            break;

        case 0xd9: // RETI
            s_coreCpuRegisterPC = coreCpuPop();
            coreBusCycle();
            s_coreCpuRegisterInterruptMasterEnable = true;
            break;

        case 0xda: // JP C, a16
            if(s_coreCpuFlagC) {
                s_coreCpuRegisterPC = coreCpuFetch16();
            } else {
                coreBusCycle();
                s_coreCpuRegisterPC += 2;
            }

            coreBusCycle();

            break;

        case 0xdc: // CALL C, a16
            if(s_coreCpuFlagC) {
                uint16_t l_operand = coreCpuFetch16();
                coreBusCycle();
                coreCpuPush(s_coreCpuRegisterPC);
                s_coreCpuRegisterPC = l_operand;
            } else {
                coreBusCycle();
                coreBusCycle();
                s_coreCpuRegisterPC += 2;
            }

            break;

        case 0xde: // SBC A, d8
            coreCpuOpSbc(coreCpuFetch8());
            break;

        case 0xdf: // RST 0x18
            coreBusCycle();
            coreCpuPush(s_coreCpuRegisterPC);
            s_coreCpuRegisterPC = 0x18;
            break;

        case 0xe0: // LDH (a8), A
            coreBusWrite(0xff00 | coreCpuFetch8(), s_coreCpuRegisterAF.byte.high);
            break;

        case 0xe1: // POP HL
            s_coreCpuRegisterHL.word = coreCpuPop();
            break;

        case 0xe2: // LDH (C), A
            coreBusWrite(0xff00 | s_coreCpuRegisterBC.byte.low, s_coreCpuRegisterAF.byte.high);
            break;

        case 0xe5: // PUSH HL
            coreBusCycle();
            coreCpuPush(s_coreCpuRegisterHL.word);
            break;

        case 0xe6: // AND d8
            coreCpuOpAnd(coreCpuFetch8());
            break;

        case 0xe7: // RST 0x20
            coreBusCycle();
            coreCpuPush(s_coreCpuRegisterPC);
            s_coreCpuRegisterPC = 0x20;
            break;

        case 0xe8: // ADD SP, r8
            s_coreCpuFlagZ = false;
            s_coreCpuFlagN = false;

            {
                uint8_t l_value = coreCpuFetch8();

                s_coreCpuFlagH = (
                    (s_coreCpuRegisterSP & 0x0f) + (l_value & 0x0f)
                ) >= 0x10;
                s_coreCpuFlagC = (
                    (s_coreCpuRegisterSP & 0xff) + l_value
                ) >= 0x100;

                s_coreCpuRegisterSP += (int8_t)l_value;
            }

            coreBusCycle();
            coreBusCycle();

            break;

        case 0xe9: // JP HL
            s_coreCpuRegisterPC = s_coreCpuRegisterHL.word;
            break;

        case 0xea: // LD (a16), A
            coreCpuWrite16(coreCpuFetch16(), s_coreCpuRegisterAF.byte.high);
            break;

        case 0xee: // XOR d8
            coreCpuOpXor(coreCpuFetch8());
            break;

        case 0xef: // RST 0x28
            coreBusCycle();
            coreCpuPush(s_coreCpuRegisterPC);
            s_coreCpuRegisterPC = 0x28;
            break;

        case 0xf0: // LDH A, (a8)
            s_coreCpuRegisterAF.byte.high = coreBusRead(0xff00 | coreCpuFetch8());
            break;

        case 0xf1: // POP AF
            s_coreCpuRegisterAF.word = coreCpuPop();
            s_coreCpuFlagZ = (s_coreCpuRegisterAF.byte.low & 0x80) != 0;
            s_coreCpuFlagN = (s_coreCpuRegisterAF.byte.low & 0x40) != 0;
            s_coreCpuFlagH = (s_coreCpuRegisterAF.byte.low & 0x20) != 0;
            s_coreCpuFlagC = (s_coreCpuRegisterAF.byte.low & 0x10) != 0;
            break;

        case 0xf2: // LDH A, (C)
            s_coreCpuRegisterAF.byte.high = coreBusRead(0xff00 | s_coreCpuRegisterBC.byte.low);
            break;

        case 0xf3: // DI
            s_coreCpuRegisterInterruptMasterEnable = false;
            s_coreCpuRegisterInterruptMasterEnableNextCycle = false;
            break;

        case 0xf5: // PUSH AF
            s_coreCpuRegisterAF.byte.low = (
                (s_coreCpuFlagZ ? 0x80 : 0x00)
                | (s_coreCpuFlagN ? 0x40 : 0x00)
                | (s_coreCpuFlagH ? 0x20 : 0x00)
                | (s_coreCpuFlagC ? 0x10 : 0x00)
            );

            coreBusCycle();
            coreCpuPush(s_coreCpuRegisterAF.word);

            break;

        case 0xf6: // OR d8
            coreCpuOpOr(coreCpuFetch8());
            break;

        case 0xf7: // RST 0x30
            coreBusCycle();
            coreCpuPush(s_coreCpuRegisterPC);
            s_coreCpuRegisterPC = 0x30;
            break;

        case 0xf8: // LD HL, SP + r8
            s_coreCpuFlagZ = false;
            s_coreCpuFlagN = false;

            coreBusCycle();

            {
                uint8_t l_value = coreCpuFetch8();

                s_coreCpuFlagH = (
                    (s_coreCpuRegisterSP & 0x0f) + (l_value & 0x0f)
                ) >= 0x10;
                s_coreCpuFlagC = (
                    (s_coreCpuRegisterSP & 0xff) + l_value
                ) >= 0x100;
            }

            break;

        case 0xf9: // LD SP, HL
            coreBusCycle();
            s_coreCpuRegisterSP = s_coreCpuRegisterHL.word;
            break;

        case 0xfa: // LD A, (a16)
            s_coreCpuRegisterAF.byte.high = coreBusRead(coreCpuFetch16());
            break;

        case 0xfb: // EI
            s_coreCpuRegisterInterruptEnable = false;
            s_coreCpuRegisterInterruptMasterEnableNextCycle = true;
            s_coreCpuCheckInterrupts = true;
            break;

        case 0xfe: // CP d8
            coreCpuOpCp(coreCpuFetch8());
            break;

        case 0xff: // RST 0x38
            coreBusCycle();
            coreCpuPush(s_coreCpuRegisterPC);
            s_coreCpuRegisterPC = 0x38;
            break;

        default: // Freeze the CPU
            s_coreCpuRegisterInterruptMasterEnableNextCycle = false;
            s_coreCpuRegisterInterruptMasterEnable = false;
            s_coreCpuRegisterPC--;
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
    coreBusWrite(--s_coreCpuRegisterSP, p_value >> 8);
    coreBusWrite(--s_coreCpuRegisterSP, p_value);
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

static inline void coreCpuOpAdc(uint8_t p_value) {
    int l_carry = s_coreCpuFlagC ? 1 : 0;

    s_coreCpuFlagN = false;
    s_coreCpuFlagH = (
        (s_coreCpuRegisterAF.byte.high & 0x0f) + (p_value & 0x0f) + l_carry
    ) >= 0x10;
    s_coreCpuFlagC = (
        s_coreCpuRegisterAF.byte.high + p_value + l_carry
    ) >= 0x100;

    s_coreCpuRegisterAF.byte.high += p_value + l_carry;

    s_coreCpuFlagZ = s_coreCpuRegisterAF.byte.high == 0;
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

static inline void coreCpuOpAnd(uint8_t p_value) {
    s_coreCpuRegisterAF.byte.high &= p_value;

    s_coreCpuFlagZ = s_coreCpuRegisterAF.byte.high == 0;
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = true;
    s_coreCpuFlagC = false;
}

static inline void coreCpuOpCp(uint8_t p_value) {
    s_coreCpuFlagN = true;
    s_coreCpuFlagH = (s_coreCpuRegisterAF.byte.high & 0x0f) < (p_value & 0x0f);
    s_coreCpuFlagC = s_coreCpuRegisterAF.byte.high < p_value;
    s_coreCpuFlagZ = s_coreCpuRegisterAF.byte.high == p_value;
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

static inline void coreCpuOpOr(uint8_t p_value) {
    s_coreCpuRegisterAF.byte.high |= p_value;

    s_coreCpuFlagZ = s_coreCpuRegisterAF.byte.high == 0;
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = false;
}

static inline uint8_t coreCpuOpRl(uint8_t p_value) {
    p_value = coreCpuOpRlBase(p_value);

    s_coreCpuFlagZ = p_value == 0;

    return p_value;
}

static inline uint8_t coreCpuOpRlBase(uint8_t p_value) {
    uint8_t l_savedBit = s_coreCpuFlagC ? 1 : 0;

    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = (p_value & 0x80) != 0;

    return (p_value << 1) | l_savedBit;
}

static inline uint8_t coreCpuOpRlc(uint8_t p_value) {
    p_value = coreCpuOpRlcBase(p_value);

    s_coreCpuFlagZ = p_value == 0;

    return p_value;
}

static inline uint8_t coreCpuOpRlcBase(uint8_t p_value) {
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = p_value >> 7;

    return (p_value << 1) | (s_coreCpuFlagC ? 1 : 0);
}

static inline uint8_t coreCpuOpRr(uint8_t p_value) {
    p_value = coreCpuOpRrBase(p_value);

    s_coreCpuFlagZ = p_value == 0;

    return p_value;
}

static inline uint8_t coreCpuOpRrBase(uint8_t p_value) {
    uint8_t l_savedBit = s_coreCpuFlagC ? 0x80 : 0;

    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = (p_value & 0x01) != 0;

    return (p_value >> 1) | l_savedBit;
}

static inline uint8_t coreCpuOpRrc(uint8_t p_value) {
    p_value = coreCpuOpRrcBase(p_value);

    s_coreCpuFlagZ = p_value == 0;

    return p_value;
}

static inline uint8_t coreCpuOpRrcBase(uint8_t p_value) {
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = (p_value & 0x01) != 0;

    return (p_value >> 1) | (s_coreCpuFlagC ? 0x80 : 0x00);
}

static inline void coreCpuOpSbc(uint8_t p_value) {
    int l_carry = s_coreCpuFlagC ? 1 : 0;

    s_coreCpuFlagN = true;
    s_coreCpuFlagH = (
        (s_coreCpuRegisterAF.byte.high & 0x0f) - (p_value & 0x0f) - l_carry
    ) < 0;
    s_coreCpuFlagC = (s_coreCpuRegisterAF.byte.high - p_value - l_carry < 0);

    s_coreCpuRegisterAF.byte.high -= p_value + l_carry;

    s_coreCpuFlagZ = s_coreCpuRegisterAF.byte.high == 0;
}

static inline uint8_t coreCpuOpSla(uint8_t p_value) {
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = (p_value & 0x80) != 0;

    p_value <<= 1;

    s_coreCpuFlagZ = p_value == 0;

    return p_value;
}

static inline uint8_t coreCpuOpSra(uint8_t p_value) {
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = (p_value & 0x01) != 0;

    p_value = ((int8_t)p_value) >> 1;

    s_coreCpuFlagZ = p_value == 0;

    return p_value;
}

static inline uint8_t coreCpuOpSrl(uint8_t p_value) {
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = (p_value & 0x01) != 0;

    p_value >>= 1;

    s_coreCpuFlagZ = p_value == 0;

    return p_value;
}

static inline void coreCpuOpSub(uint8_t p_value) {
    s_coreCpuFlagN = true;
    s_coreCpuFlagH = (s_coreCpuRegisterAF.byte.high & 0x0f) < (p_value & 0x0f);
    s_coreCpuFlagC = s_coreCpuRegisterAF.byte.high < p_value;

    s_coreCpuRegisterAF.byte.high -= p_value;

    s_coreCpuFlagZ = s_coreCpuRegisterAF.byte.high == 0;
}

static inline uint8_t coreCpuOpSwap(uint8_t p_value) {
    s_coreCpuFlagZ = p_value == 0;
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = false;

    return ((p_value & 0x0f) << 4) | (p_value >> 4);
}

static inline void coreCpuOpXor(uint8_t p_value) {
    s_coreCpuRegisterAF.byte.high ^= p_value;

    s_coreCpuFlagZ = s_coreCpuRegisterAF.byte.high == 0;
    s_coreCpuFlagN = false;
    s_coreCpuFlagH = false;
    s_coreCpuFlagC = false;
}
