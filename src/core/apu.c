#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "core/apu.h"

#define C_SAMPLES_PER_SECOND 262144
#define C_BUFFER_LENGTH (C_SAMPLES_PER_SECOND / 16)

static const uint8_t s_coreApuWaveRamInitialValues[16] = {
    0x84, 0x40, 0x43, 0xaa,
    0x2d, 0x78, 0x92, 0x3c,
    0x60, 0x59, 0x59, 0xb0,
    0x34, 0xb8, 0x2e, 0xda
};
static float s_coreApuBuffer[2][C_SAMPLES_PER_SECOND];
static int s_coreApuBufferLength;
static int s_coreApuDiv;
static uint8_t s_coreApuRegisterNr10;
static uint8_t s_coreApuRegisterNr11;
static uint8_t s_coreApuRegisterNr12;
static bool s_coreApuSoundLengthEnable1;
static uint8_t s_coreApuRegisterNr21;
static uint8_t s_coreApuRegisterNr22;
static bool s_coreApuSoundLengthEnable2;
static bool s_coreApuSoundChannelDac3;
static uint8_t s_coreApuRegisterNr32;
static bool s_coreApuSoundLengthEnable3;
static uint8_t s_coreApuRegisterNr42;
static uint8_t s_coreApuRegisterNr43;
static bool s_coreApuSoundLengthEnable4;
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

static inline void coreApuUpdateLengthCounter(void);
static inline void coreApuUpdateVolumeEnvelope(void);
static inline void coreApuUpdateSweep(void);

void coreApuReset(void) {
    coreApuWriteIo(0xff24, 0x00);
    coreApuWriteIo(0xff25, 0x00);
    s_coreApuRegisterNr52 = 0x70;
    s_coreApuOn = false;
    s_coreApuChannelOn1 = false;
    s_coreApuChannelOn2 = false;
    s_coreApuChannelOn3 = false;
    s_coreApuChannelOn4 = false;

    memcpy(s_coreApuRegisterWavePattern, s_coreApuWaveRamInitialValues, 16);
}

void coreApuCycle(void) {
    
}

void coreApuDivCycle(void) {
    s_coreApuDiv++;

    bool l_updateLengthCounter = (s_coreApuDiv & 0x01) != 0;
    bool l_updateEnvelope = (s_coreApuDiv & 0x07) == 0x07;
    bool l_updateSweep = (s_coreApuDiv & 0x03) == 0x02;

    if(l_updateLengthCounter) {
        coreApuUpdateLengthCounter();
    }

    if(l_updateEnvelope) {
        coreApuUpdateVolumeEnvelope();
    }

    if(l_updateSweep) {
        coreApuUpdateSweep();
    }
}

uint8_t coreApuReadIo(uint16_t p_address) {
    if(p_address == 0xff10) {
        return s_coreApuRegisterNr10 | 0x80;
    } else if(p_address == 0xff11) {
        return s_coreApuRegisterNr11 | 0x3f;
    } else if(p_address == 0xff12) {
        return s_coreApuRegisterNr12;
    } else if(p_address == 0xff14) {
        if(s_coreApuSoundLengthEnable1) {
            return 0xff;
        } else {
            return 0xbf;
        }
    } else if(p_address == 0xff16) {
        return s_coreApuRegisterNr21 | 0x3f;
    } else if(p_address == 0xff17) {
        return s_coreApuRegisterNr22;
    } else if(p_address == 0xff19) {
        if(s_coreApuSoundLengthEnable2) {
            return 0xff;
        } else {
            return 0xbf;
        }
    } else if(p_address == 0xff1a) {
        if(s_coreApuSoundChannelDac3) {
            return 0xff;
        } else {
            return 0x7f;
        }
    } else if(p_address == 0xff1c) {
        return s_coreApuRegisterNr32 | 0x9f;
    } else if(p_address == 0xff1e) {
        if(s_coreApuSoundLengthEnable3) {
            return 0xff;
        } else {
            return 0xbf;
        }
    } else if(p_address == 0xff21) {
        return s_coreApuRegisterNr42;
    } else if(p_address == 0xff22) {
        return s_coreApuRegisterNr43;
    } else if(p_address == 0xff23) {
        if(s_coreApuSoundLengthEnable4) {
            return 0xff;
        } else {
            return 0xbf;
        }
    } else if(p_address == 0xff24) {
        return s_coreApuRegisterNr50;
    } else if(p_address == 0xff25) {
        return s_coreApuRegisterNr51;
    } else if(p_address == 0xff26) {
        uint8_t l_value = s_coreApuRegisterNr52 & 0x80;

        if(s_coreApuChannelOn1) {
            l_value |= 0x01;
        }

        if(s_coreApuChannelOn2) {
            l_value |= 0x02;
        }

        if(s_coreApuChannelOn3) {
            l_value |= 0x04;
        }

        if(s_coreApuChannelOn4) {
            l_value |= 0x08;
        }

        return l_value;
    } else {
        return 0xff;
    }
}

uint8_t coreApuReadWavePattern(uint16_t p_address) {
    return s_coreApuRegisterWavePattern[p_address & 0x000f];
}

void coreApuWriteIo(uint16_t p_address, uint8_t p_value) {
    if(p_address == 0xff10) {
        s_coreApuRegisterNr10 = p_value;
    } else if(p_address == 0xff11) {
        s_coreApuRegisterNr11 = p_value;
    } else if(p_address == 0xff12) {
        s_coreApuRegisterNr12 = p_value;
    } else if(p_address == 0xff13) {
        
    } else if(p_address == 0xff14) {
        s_coreApuSoundLengthEnable1 = (p_value & 0x40) != 0;
    } else if(p_address == 0xff16) {
        s_coreApuRegisterNr21 = p_value;
    } else if(p_address == 0xff17) {
        s_coreApuRegisterNr22 = p_value;
    } else if(p_address == 0xff18) {

    } else if(p_address == 0xff19) {
        s_coreApuSoundLengthEnable2 = (p_value & 0x40) != 0;
    } else if(p_address == 0xff1a) {
        s_coreApuSoundChannelDac3 = (p_value & 0x80) != 0;
    } else if(p_address == 0xff1b) {

    } else if(p_address == 0xff1c) {
        s_coreApuRegisterNr32 = p_value;
    } else if(p_address == 0xff1d) {

    } else if(p_address == 0xff1e) {
        s_coreApuSoundLengthEnable3 = (p_value & 0x40) != 0;
    } else if(p_address == 0xff20) {

    } else if(p_address == 0xff21) {
        s_coreApuRegisterNr42 = p_value;
    } else if(p_address == 0xff22) {
        s_coreApuRegisterNr43 = p_value;
    } else if(p_address == 0xff23) {
        s_coreApuSoundLengthEnable4 = (p_value & 0x40) != 0;
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

static inline void coreApuUpdateLengthCounter(void) {

}

static inline void coreApuUpdateVolumeEnvelope(void) {

}

static inline void coreApuUpdateSweep(void) {

}
