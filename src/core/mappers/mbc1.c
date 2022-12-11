#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "core/cartridge.h"
#include "core/mappers/mbc1.h"

static const uint8_t *s_romData;
static size_t s_romSize;
static uint8_t *s_ramData;
static size_t s_ramSize;
static uint16_t s_ramAddressMask;
static int s_romBankNumber;
static int s_ramBankNumber;
static int s_romOffsetLow;
static int s_romOffsetHigh;
static int s_bankingMode;
static bool s_ramEnabled;

static int coreMapperMbc1Init(
    const uint8_t *p_romData,
    size_t p_romSize,
    uint8_t *p_ramData,
    size_t p_ramSize
);
static void coreMapperMbc1Reset(void);
static uint8_t coreMapperMbc1ReadRom(uint16_t p_address);
static uint8_t coreMapperMbc1ReadRam(uint16_t p_address);
static void coreMapperMbc1WriteRom(uint16_t p_address, uint8_t p_value);
static void coreMapperMbc1WriteRam(uint16_t p_address, uint8_t p_value);
static void coreMapperMbc1RefreshBankOffset(void);

const struct ts_coreCartridgeMapper g_coreCartridgeMapperMbc1 = {
    .name = "MBC1",
    .init = coreMapperMbc1Init,
    .reset = coreMapperMbc1Reset,
    .readRam = coreMapperMbc1ReadRam,
    .readRom = coreMapperMbc1ReadRom,
    .writeRam = coreMapperMbc1WriteRam,
    .writeRom = coreMapperMbc1WriteRom
};

static int coreMapperMbc1Init(
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

    return 0;
}

static void coreMapperMbc1Reset(void) {
    s_romBankNumber = 0;
    s_ramBankNumber = 0;
    s_bankingMode = 0;
    s_ramEnabled = false;
    coreMapperMbc1RefreshBankOffset();
}

static uint8_t coreMapperMbc1ReadRom(uint16_t p_address) {
    int l_address = p_address & 0x3fff;

    if((p_address & 0x4000) == 0) {
        l_address |= s_romOffsetLow;
    } else {
        l_address |= s_romOffsetHigh;
    }

    return s_romData[l_address];
}

static uint8_t coreMapperMbc1ReadRam(uint16_t p_address) {
    // TODO
    return 0xff;
}

static void coreMapperMbc1WriteRom(uint16_t p_address, uint8_t p_value) {
    switch(p_address & 0x6000) {
        case 0x0000:
            s_ramEnabled = (p_value == 0x0a);
            break;

        case 0x2000:
            s_romBankNumber = p_value;
            coreMapperMbc1RefreshBankOffset();
            break;

        case 0x4000:
            s_ramBankNumber = p_value;
            coreMapperMbc1RefreshBankOffset();
            break;

        case 0x6000:
            s_bankingMode = p_value;
            coreMapperMbc1RefreshBankOffset();
            break;
    }
}

static void coreMapperMbc1WriteRam(uint16_t p_address, uint8_t p_value) {
    // TODO
}

static void coreMapperMbc1RefreshBankOffset(void) {
    s_romOffsetLow = 0x0000;
    s_romOffsetHigh = 0x4000;

    // TODO
}
