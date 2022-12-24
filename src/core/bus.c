#include <stdint.h>

#include "common.h"
#include "core/apu.h"
#include "core/bios.h"
#include "core/bus.h"
#include "core/cartridge.h"
#include "core/core.h"
#include "core/cpu.h"
#include "core/dma.h"
#include "core/hram.h"
#include "core/joypad.h"
#include "core/ppu.h"
#include "core/serial.h"
#include "core/timer.h"
#include "core/wram.h"

typedef uint8_t (*t_coreBusReadFunc)(uint16_t p_address);
typedef void (*t_coreBusWriteFunc)(uint16_t p_address, uint8_t p_value);

static t_coreBusReadFunc s_coreBusReadFuncTable[65536];
static t_coreBusWriteFunc s_coreBusWriteFuncTable[65536];
static bool s_coreBusDoubleSpeed;

static uint8_t coreBusReadOpenBus(uint16_t p_address);
static void coreBusWriteOpenBus(uint16_t p_address, uint8_t p_value);
static void coreBusWriteBiosDisable(uint16_t p_address, uint8_t p_value);

void coreBusReset(void) {
    // Default: read open bus
    for(int i = 0; i < 65536; i++) {
        s_coreBusReadFuncTable[i] = coreBusReadOpenBus;
        s_coreBusWriteFuncTable[i] = coreBusWriteOpenBus;
    }

    // BIOS
    for(uint16_t l_address = 0x0000; l_address <= 0x00ff; l_address++) {
        s_coreBusReadFuncTable[l_address] = coreBiosRead;
    }

    // Cartridge ROM
    const struct ts_coreCartridgeMapper *l_mapper = coreCartridgeGetMapper();

    for(uint16_t l_address = 0x0100; l_address <= 0x7fff; l_address++) {
        s_coreBusReadFuncTable[l_address] = l_mapper->readRom;
        s_coreBusWriteFuncTable[l_address] = l_mapper->writeRom;
    }

    // PPU VRAM
    for(uint16_t l_address = 0x8000; l_address <= 0x9fff; l_address++) {
        s_coreBusReadFuncTable[l_address] = corePpuReadVram;
        s_coreBusWriteFuncTable[l_address] = corePpuWriteVram;
    }

    // Cartridge SRAM
    for(uint16_t l_address = 0xa000; l_address <= 0xbfff; l_address++) {
        s_coreBusReadFuncTable[l_address] = l_mapper->readRam;
        s_coreBusWriteFuncTable[l_address] = l_mapper->writeRam;
    }

    // WRAM
    for(uint16_t l_address = 0xc000; l_address <= 0xfdff; l_address++) {
        s_coreBusReadFuncTable[l_address] = coreWramRead;
        s_coreBusWriteFuncTable[l_address] = coreWramWrite;
    }

    // PPU OAM
    for(uint16_t l_address = 0xfe00; l_address <= 0xfe9f; l_address++) {
        s_coreBusReadFuncTable[l_address] = corePpuReadOam;
        s_coreBusWriteFuncTable[l_address] = corePpuWriteOam;
    }

    // Joypad
    s_coreBusReadFuncTable[0xff00] = coreJoypadRead;
    s_coreBusWriteFuncTable[0xff00] = coreJoypadWrite;

    // Serial
    for(uint16_t l_address = 0xff01; l_address <= 0xff02; l_address++) {
        s_coreBusReadFuncTable[l_address] = coreSerialRead;
        s_coreBusWriteFuncTable[l_address] = coreSerialWrite;
    }

    // Timer
    for(uint16_t l_address = 0xff04; l_address <= 0xff07; l_address++) {
        s_coreBusReadFuncTable[l_address] = coreTimerRead;
        s_coreBusWriteFuncTable[l_address] = coreTimerWrite;
    }

    // CPU IF
    s_coreBusReadFuncTable[0xff0f] = coreCpuRead;
    s_coreBusWriteFuncTable[0xff0f] = coreCpuWrite;

    // APU I/O
    for(uint16_t l_address = 0xff10; l_address <= 0xff26; l_address++) {
        s_coreBusReadFuncTable[l_address] = coreApuReadIo;
        s_coreBusWriteFuncTable[l_address] = coreApuWriteIo;
    }

    // APU wave pattern
    for(uint16_t l_address = 0xff30; l_address <= 0xff3f; l_address++) {
        s_coreBusReadFuncTable[l_address] = coreApuReadWavePattern;
        s_coreBusWriteFuncTable[l_address] = coreApuWriteWavePattern;
    }

    // PPU I/O
    for(uint16_t l_address = 0xff40; l_address <= 0xff45; l_address++) {
        s_coreBusReadFuncTable[l_address] = corePpuReadIo;
        s_coreBusWriteFuncTable[l_address] = corePpuWriteIo;
    }

    // OAM DMA
    s_coreBusReadFuncTable[0xff46] = coreDmaRead;
    s_coreBusWriteFuncTable[0xff46] = coreDmaWrite;

    // PPU I/O
    for(uint16_t l_address = 0xff47; l_address <= 0xff4b; l_address++) {
        s_coreBusReadFuncTable[l_address] = corePpuReadIo;
        s_coreBusWriteFuncTable[l_address] = corePpuWriteIo;
    }

    // BIOS disable
    s_coreBusWriteFuncTable[0xff50] = coreBusWriteBiosDisable;

    // HRAM
    for(uint16_t l_address = 0xff80; l_address <= 0xfffe; l_address++) {
        s_coreBusReadFuncTable[l_address] = coreHramRead;
        s_coreBusWriteFuncTable[l_address] = coreHramWrite;
    }

    // CPU IE
    s_coreBusReadFuncTable[0xffff] = coreCpuRead;
    s_coreBusWriteFuncTable[0xffff] = coreCpuWrite;

    // CPU starts in simple speed
    s_coreBusDoubleSpeed = false;
}

uint8_t coreBusRead(uint16_t p_address) {
    uint8_t l_returnValue;

    if(coreDmaIsRunning() && ((p_address & 0xff80) != 0xff80)) {
        l_returnValue = 0xff;
    } else {
        l_returnValue = s_coreBusReadFuncTable[p_address](p_address);
    }

    coreBusCycle();

    return l_returnValue;
}

uint8_t coreBusReadDma(uint16_t p_address) {
    return s_coreBusReadFuncTable[p_address](p_address);
}

void coreBusWrite(uint16_t p_address, uint8_t p_value) {
    if(!coreDmaIsRunning() || ((p_address & 0xff80) == 0xff80)) {
        s_coreBusWriteFuncTable[p_address](p_address, p_value);
    }

    coreBusCycle();
}

void coreBusCycle(void) {
    if(s_coreBusDoubleSpeed) {
        coreCartridgeCycleDouble();
        corePpuCycleDouble();
    } else {
        coreCartridgeCycle();
        corePpuCycle();
    }

    coreDmaCycle();
    coreSerialCycle();
    coreTimerCycle();
}

void coreBusSetDoubleSpeed(bool p_doubleSpeed) {
    s_coreBusDoubleSpeed = p_doubleSpeed;
}

static uint8_t coreBusReadOpenBus(uint16_t p_address) {
    M_UNUSED_PARAMETER(p_address);

    return 0xff;
}

static void coreBusWriteOpenBus(uint16_t p_address, uint8_t p_value) {
    M_UNUSED_PARAMETER(p_address);
    M_UNUSED_PARAMETER(p_value);

    // Ignore the write
}

static void coreBusWriteBiosDisable(uint16_t p_address, uint8_t p_value) {
    // The address is unused.
    M_UNUSED_PARAMETER(p_address);

    if(p_value != 0) {
        const struct ts_coreCartridgeMapper *l_mapper = coreCartridgeGetMapper();

        for(uint16_t l_address = 0x0000; l_address <= 0x00ff; l_address++) {
            s_coreBusReadFuncTable[l_address] = l_mapper->readRom;
            s_coreBusWriteFuncTable[l_address] = l_mapper->writeRom;
        }
    }
}
