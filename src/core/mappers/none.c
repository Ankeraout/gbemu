#include <stdint.h>

#include "common.h"
#include "core/cartridge.h"
#include "core/mappers/none.h"

static const uint8_t *s_romData;
static size_t s_romSize;
static uint8_t *s_ramData;
static size_t s_ramSize;
static uint16_t s_ramAddressMask;

static int coreMapperNoneInit(
    const uint8_t *p_romData,
    size_t p_romSize,
    uint8_t *p_ramData,
    size_t p_ramSize
);
static uint8_t coreMapperNoneReadRom(uint16_t p_address);
static uint8_t coreMapperNoneReadRam(uint16_t p_address);
static void coreMapperNoneWriteRom(uint16_t p_address, uint8_t p_value);
static void coreMapperNoneWriteRam(uint16_t p_address, uint8_t p_value);

const struct ts_coreCartridgeMapper g_coreCartridgeMapperNone = {
    .name = "None",
    .init = coreMapperNoneInit,
    .reset = NULL,
    .cycle = NULL,
    .readRam = coreMapperNoneReadRam,
    .readRom = coreMapperNoneReadRom,
    .writeRam = coreMapperNoneWriteRam,
    .writeRom = coreMapperNoneWriteRom
};

static int coreMapperNoneInit(
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

static uint8_t coreMapperNoneReadRom(uint16_t p_address) {
    return s_romData[p_address & 0x7fff];
}

static uint8_t coreMapperNoneReadRam(uint16_t p_address) {
    if(s_ramSize != 0) {
        return s_ramData[p_address & s_ramAddressMask];
    } else {
        return 0xff;
    }
}

static void coreMapperNoneWriteRom(uint16_t p_address, uint8_t p_value) {
    M_UNUSED_PARAMETER(p_address);
    M_UNUSED_PARAMETER(p_value);
}

static void coreMapperNoneWriteRam(uint16_t p_address, uint8_t p_value) {
    if(s_ramSize != 0) {
        s_ramData[p_address & s_ramAddressMask] = p_value;
    }
}
