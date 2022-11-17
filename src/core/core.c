#include <stdint.h>

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

int coreInit(void) {
    return 0;
}

void coreSetBios(const void *p_bios) {
    coreBiosSetBios(p_bios);
}

int coreSetRom(const void *p_rom, size_t p_size) {
    return coreCartridgeSetRom(p_rom, p_size);
}

void coreReset(void) {
    coreBusReset();
    coreCartridgeReset();
    coreCpuReset();
    coreDmaReset();
    coreHramReset();
    coreJoypadReset();
    corePpuReset();
    coreSerialReset();
    coreTimerReset();
    coreWramReset();
}

void coreFrameAdvance(void) {

}

void coreStep(void) {

}
