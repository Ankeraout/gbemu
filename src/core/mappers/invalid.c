#include <stdint.h>

#include "common.h"
#include "core/cartridge.h"
#include "core/mappers/invalid.h"

static int coreMapperInvalidInit(
    const uint8_t *p_romData,
    size_t p_romSize,
    uint8_t *p_ramData,
    size_t p_ramSize
);
static void coreMapperInvalidReset(void);
static uint8_t coreMapperInvalidReadRom(uint16_t p_address);
static uint8_t coreMapperInvalidReadRam(uint16_t p_address);
static void coreMapperInvalidWriteRom(uint16_t p_address, uint8_t p_value);
static void coreMapperInvalidWriteRam(uint16_t p_address, uint8_t p_value);

const struct ts_coreCartridgeMapper g_coreCartridgeMapperInvalid = {
    .name = "Invalid",
    .init = coreMapperInvalidInit,
    .reset = coreMapperInvalidReset,
    .readRam = coreMapperInvalidReadRam,
    .readRom = coreMapperInvalidReadRom,
    .writeRam = coreMapperInvalidWriteRam,
    .writeRom = coreMapperInvalidWriteRom
};

static int coreMapperInvalidInit(
    const uint8_t *p_romData,
    size_t p_romSize,
    uint8_t *p_ramData,
    size_t p_ramSize
) {
    M_UNUSED_PARAMETER(p_romData);
    M_UNUSED_PARAMETER(p_romSize);
    M_UNUSED_PARAMETER(p_ramData);
    M_UNUSED_PARAMETER(p_ramSize);

    return 0;
}

static void coreMapperInvalidReset(void) {

}

static uint8_t coreMapperInvalidReadRom(uint16_t p_address) {
    M_UNUSED_PARAMETER(p_address);

    return 0xff;
}

static uint8_t coreMapperInvalidReadRam(uint16_t p_address) {
    M_UNUSED_PARAMETER(p_address);

    return 0xff;
}

static void coreMapperInvalidWriteRom(uint16_t p_address, uint8_t p_value) {
    M_UNUSED_PARAMETER(p_address);
    M_UNUSED_PARAMETER(p_value);
}

static void coreMapperInvalidWriteRam(uint16_t p_address, uint8_t p_value) {
    M_UNUSED_PARAMETER(p_address);
    M_UNUSED_PARAMETER(p_value);
}
