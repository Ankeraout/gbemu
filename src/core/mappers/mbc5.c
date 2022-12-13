#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "core/mappers/mbc3.h"

static const uint8_t *s_romData;
static size_t s_romSize;
static uint8_t *s_ramData;
static size_t s_ramSize;
static uint16_t s_ramAddressMask;
static uint32_t s_romAddressMask;
static uint32_t s_romOffset;
static uint32_t s_ramOffset;
static bool s_ramEnabled;

static int coreMapperMbc5Init(
    const uint8_t *p_romData,
    size_t p_romSize,
    uint8_t *p_ramData,
    size_t p_ramSize
);
static void coreMapperMbc5Reset(void);
static uint8_t coreMapperMbc5ReadRom(uint16_t p_address);
static uint8_t coreMapperMbc5ReadRam(uint16_t p_address);
static void coreMapperMbc5WriteRom(uint16_t p_address, uint8_t p_value);
static void coreMapperMbc5WriteRam(uint16_t p_address, uint8_t p_value);

const struct ts_coreCartridgeMapper g_coreCartridgeMapperMbc5 = {
    .name = "MBC5",
    .init = coreMapperMbc5Init,
    .reset = coreMapperMbc5Reset,
    .cycle = NULL,
    .readRam = coreMapperMbc5ReadRam,
    .readRom = coreMapperMbc5ReadRom,
    .writeRam = coreMapperMbc5WriteRam,
    .writeRom = coreMapperMbc5WriteRom
};

static int coreMapperMbc5Init(
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
    coreMapperMbc5Reset();

    return 0;
}

static void coreMapperMbc5Reset(void) {
    s_ramOffset = 0x0000;
    s_romOffset = 0x4000;
    s_ramEnabled = false;
}

static uint8_t coreMapperMbc5ReadRom(uint16_t p_address) {
    if((p_address & 0x4000) == 0) {
        return s_romData[p_address];
    } else {
        return s_romData[s_romOffset | (p_address & 0x3fff)];
    }
}

static uint8_t coreMapperMbc5ReadRam(uint16_t p_address) {
    if(s_ramEnabled) {
        return s_ramData[s_ramOffset | (p_address & 0x1fff)];
    } else {
        return 0xff;
    }
}

static void coreMapperMbc5WriteRom(uint16_t p_address, uint8_t p_value) {
    switch(p_address & 0x7000) {
        case 0x0000: case 0x1000: s_ramEnabled = p_value == 0x0a; break;
        case 0x2000:
            s_romOffset &= 0x00400000;
            s_romOffset |= p_value << 14;
            break;

        case 0x3000:
            s_romOffset &= 0x003fc000;
            s_romOffset |= (p_value & 0x01) << 22;
            break;

        case 0x4000: case 0x5000:
            s_ramOffset = (p_value & 0x0f) << 13;
            break;

        default:
            break;
    }
}

static void coreMapperMbc5WriteRam(uint16_t p_address, uint8_t p_value) {
    if(s_ramEnabled) {
        s_ramData[s_ramOffset | (p_address & 0x1fff)] = p_value;;
    }
}
