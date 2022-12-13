#include <stdbool.h>
#include <stdint.h>

#include "core/apu.h"

static uint8_t s_coreApuRegisterNr50;
static uint8_t s_coreApuRegisterNr51;
static uint8_t s_coreApuRegisterNr52;
static uint8_t s_coreApuRegisterWavePattern[16];
static bool s_coreApuOn;
static bool s_coreApuChannelOn1;
static bool s_coreApuChannelOn2;
static bool s_coreApuChannelOn3;
static bool s_coreApuChannelOn4;
static bool s_coreApuChannelMixLeft1;
static bool s_coreApuChannelMixLeft2;
static bool s_coreApuChannelMixLeft3;
static bool s_coreApuChannelMixLeft4;
static bool s_coreApuChannelMixRight1;
static bool s_coreApuChannelMixRight2;
static bool s_coreApuChannelMixRight3;
static bool s_coreApuChannelMixRight4;
static float s_coreApuVolumeLeft;
static float s_coreApuVolumeRight;

void coreApuReset(void) {

}

void coreApuCycle(void) {

}

uint8_t coreApuReadIo(uint16_t p_address) {
    return 0xff;
}

uint8_t coreApuReadWavePattern(uint16_t p_address) {
    return s_coreApuRegisterWavePattern[p_address & 0x000f];
}

void coreApuWriteIo(uint16_t p_address, uint8_t p_value) {
    if(p_address == 0xff10) {

    } else if(p_address == 0xff11) {

    } else if(p_address == 0xff12) {

    } else if(p_address == 0xff13) {

    } else if(p_address == 0xff14) {

    } else if(p_address == 0xff16) {

    } else if(p_address == 0xff17) {

    } else if(p_address == 0xff18) {

    } else if(p_address == 0xff19) {

    } else if(p_address == 0xff1a) {

    } else if(p_address == 0xff1b) {

    } else if(p_address == 0xff1c) {

    } else if(p_address == 0xff1d) {

    } else if(p_address == 0xff1e) {

    } else if(p_address == 0xff20) {

    } else if(p_address == 0xff21) {

    } else if(p_address == 0xff22) {

    } else if(p_address == 0xff23) {

    } else if(p_address == 0xff24) {
        s_coreApuRegisterNr50 = p_value;

        if((p_value & 0x80) != 0) {
            s_coreApuVolumeLeft = ((p_value & 0x70) >> 4) / 8.f;
        } else {
            s_coreApuVolumeLeft = 0.f;
        }

        if((p_value & 0x08) != 0) {
            s_coreApuVolumeRight = ((p_value & 0x70) >> 4) / 8.f;
        } else {
            s_coreApuVolumeRight = 0.f;
        }
    } else if(p_address == 0xff25) {
        s_coreApuRegisterNr51 = p_value;
        s_coreApuChannelMixLeft4 = (p_value & 0x80) != 0;
        s_coreApuChannelMixLeft3 = (p_value & 0x40) != 0;
        s_coreApuChannelMixLeft2 = (p_value & 0x20) != 0;
        s_coreApuChannelMixLeft1 = (p_value & 0x10) != 0;
        s_coreApuChannelMixRight4 = (p_value & 0x08) != 0;
        s_coreApuChannelMixRight3 = (p_value & 0x04) != 0;
        s_coreApuChannelMixRight2 = (p_value & 0x02) != 0;
        s_coreApuChannelMixRight1 = (p_value & 0x01) != 0;
    } else if(p_address == 0xff26) {
        s_coreApuRegisterNr52 = p_value;
        s_coreApuOn = (p_value & 0x80) != 0;
    }
}

void coreApuWriteWavePattern(uint16_t p_address, uint8_t p_value) {
    s_coreApuRegisterWavePattern[p_address & 0x000f] = p_value;
}
