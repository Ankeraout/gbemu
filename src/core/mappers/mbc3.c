#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "core/mappers/mbc3.h"

struct ts_coreMapperMbc3Clock {
    uint8_t a_seconds;
    uint8_t a_minutes;
    uint8_t a_hours;
    uint16_t a_day;
    bool a_halt;
    bool a_overflow;
};

static const uint8_t *s_romData;
static size_t s_romSize;
static uint8_t *s_ramData;
static size_t s_ramSize;
static uint16_t s_ramAddressMask;
static uint32_t s_romAddressMask;
static uint32_t s_romOffset;
static uint32_t s_ramOffset;
static bool s_ramEnabled;
static bool s_ramSelected;
static bool s_latch;
static struct ts_coreMapperMbc3Clock s_clock;
static struct ts_coreMapperMbc3Clock s_latchedClock;
static int s_cycleCounter;

static int coreMapperMbc3Init(
    const uint8_t *p_romData,
    size_t p_romSize,
    uint8_t *p_ramData,
    size_t p_ramSize
);
static void coreMapperMbc3Reset(void);
static void coreMapperMbc3Cycle(void);
static void coreMapperMbc3CycleDouble(void);
static uint8_t coreMapperMbc3ReadRom(uint16_t p_address);
static uint8_t coreMapperMbc3ReadRam(uint16_t p_address);
static void coreMapperMbc3WriteRom(uint16_t p_address, uint8_t p_value);
static void coreMapperMbc3WriteRam(uint16_t p_address, uint8_t p_value);

const struct ts_coreCartridgeMapper g_coreCartridgeMapperMbc3 = {
    .name = "MBC3",
    .init = coreMapperMbc3Init,
    .reset = coreMapperMbc3Reset,
    .cycle = coreMapperMbc3Cycle,
    .cycleDouble = coreMapperMbc3CycleDouble,
    .readRam = coreMapperMbc3ReadRam,
    .readRom = coreMapperMbc3ReadRom,
    .writeRam = coreMapperMbc3WriteRam,
    .writeRom = coreMapperMbc3WriteRom
};

static int coreMapperMbc3Init(
    const uint8_t *p_romData,
    size_t p_romSize,
    uint8_t *p_ramData,
    size_t p_ramSize
) {
    s_romData = p_romData;
    s_romSize = p_romSize;
    s_ramData = p_ramData;
    s_ramSize = p_ramSize;
    s_ramAddressMask = p_ramSize - 1;
    s_romAddressMask = p_romSize - 1;
    coreMapperMbc3Reset();

    return 0;
}

static void coreMapperMbc3Reset(void) {
    s_ramOffset = 0x0000;
    s_romOffset = 0x4000;
    s_ramEnabled = false;
    s_ramSelected = true;
    s_latch = false;
    s_clock.a_seconds = 0;
    s_clock.a_minutes = 0;
    s_clock.a_hours = 0;
    s_clock.a_day = 0;
    s_clock.a_halt = false;
    s_clock.a_overflow = false;
}

static void coreMapperMbc3Cycle(void) {
    s_cycleCounter++;

    if((s_cycleCounter & 0x000fffff) == 0) {
        s_clock.a_seconds++;

        if(s_clock.a_seconds >= 60) {
            s_clock.a_minutes++;
            s_clock.a_seconds = 0;

            if(s_clock.a_minutes >= 60) {
                s_clock.a_hours++;
                s_clock.a_minutes = 0;

                if(s_clock.a_hours >= 24) {
                    s_clock.a_day++;
                    s_clock.a_hours = 0;

                    if(s_clock.a_day >= 512) {
                        s_clock.a_day &= 0x1ff;
                        s_clock.a_overflow = true;
                    }
                }
            }
        }
    }
}

static void coreMapperMbc3CycleDouble(void) {
    // TODO
}

static uint8_t coreMapperMbc3ReadRom(uint16_t p_address) {
    if((p_address & 0x4000) == 0) {
        return s_romData[p_address];
    } else {
        return s_romData[(s_romOffset | (p_address & 0x3fff)) & s_romAddressMask];
    }
}

static uint8_t coreMapperMbc3ReadRam(uint16_t p_address) {
    if(!s_ramEnabled) {
        return 0xff;
    }

    if(s_ramSelected) {
        return s_ramData[(s_ramOffset | (p_address & 0x1fff)) & s_ramAddressMask];
    } else {
        struct ts_coreMapperMbc3Clock *l_clockSource;

        if(s_latch) {
            l_clockSource = &s_latchedClock;
        } else {
            l_clockSource = &s_clock;
        }

        switch(s_ramOffset) {
            case 0x08: return l_clockSource->a_seconds;
            case 0x09: return l_clockSource->a_minutes;
            case 0x0a: return l_clockSource->a_hours;
            case 0x0b: return l_clockSource->a_day;
            case 0x0c:
                {
                    uint8_t l_returnedValue = l_clockSource->a_day >> 7;

                    if(l_clockSource->a_halt) {
                        l_returnedValue |= 0x40;
                    }

                    if(l_clockSource->a_overflow) {
                        l_returnedValue |= 0x80;
                    }

                    return l_returnedValue;
                }

            default:
                return 0xff;
        }
    }
}

static void coreMapperMbc3WriteRom(uint16_t p_address, uint8_t p_value) {
    switch(p_address & 0x6000) {
        case 0x0000: s_ramEnabled = ((p_value == 0x0a) && (s_ramSize != 0)); break;
        case 0x2000:
            if(p_value == 0) {
                s_romOffset = 0x4000;
            } else {
                s_romOffset = (p_value & 0x7f) << 14;
            }

            break;

        case 0x4000:
            if(p_value <= 3) {
                s_ramSelected = true;
                s_ramOffset = p_value << 13;
            } else {
                s_ramSelected = false;
                s_ramOffset = p_value;
            }

            break;

        case 0x6000:
            {
                bool l_latch = (p_value & 0x01) != 0;

                if(l_latch && (!s_latch)) {
                    memcpy(&s_latchedClock, &s_clock, sizeof(s_clock));
                }

                s_latch = l_latch;
            }

            break;
    }
}

static void coreMapperMbc3WriteRam(uint16_t p_address, uint8_t p_value) {
    if(!s_ramEnabled) {
        return;
    }

    if(s_ramSelected) {
        s_ramData[(s_ramOffset | (p_address & 0x1fff)) & s_ramAddressMask] = p_value;
    } else {
        switch(s_ramOffset) {
            case 0x08: s_clock.a_seconds = p_value & 0x3f; break;
            case 0x09: s_clock.a_minutes = p_value & 0x3f; break;
            case 0x0a: s_clock.a_hours = p_value & 0x1f; break;
            case 0x0b:
                s_clock.a_day &= 0xff00;
                s_clock.a_day |= p_value;
                break;

            case 0x0c:
                s_clock.a_day &= 0x00ff;
                s_clock.a_day |= (p_value & 0x01) << 8;
                s_clock.a_halt = (p_value & 0x40) != 0;
                s_clock.a_overflow = (p_value & 0x80) != 0;
                break;

            default:
                break;
        }
    }
}
