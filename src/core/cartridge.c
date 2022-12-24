#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/cartridge.h"
#include "core/mappers/invalid.h"
#include "core/mappers/mbc1.h"
#include "core/mappers/mbc3.h"
#include "core/mappers/mbc5.h"
#include "core/mappers/none.h"

static const uint8_t *s_coreCartridgeRomData;
static size_t s_coreCartridgeRomSize;
static uint8_t *s_coreCartridgeSramData;
static size_t s_coreCartridgeSramSize;
static const struct ts_coreCartridgeMapper *s_coreCartridgeMapper;

static uint8_t coreCartridgeComputeHeaderChecksum(void);
static uint16_t coreCartridgeComputeGlobalChecksum(void);
static size_t coreCartridgeComputeRomSize(void);
static size_t coreCartridgeComputeRamSize(void);
static const struct ts_coreCartridgeMapper *coreCartridgeDetermineMapper(void);
static bool coreCartridgeIsHeaderChecksumCorrect(void);
static bool coreCartridgeIsGlobalChecksumCorrect(void);
static bool coreCartridgeIsRomSizeCorrect(void);
static bool coreCartridgeIsRamSizeCorrect(void);

void coreCartridgeReset(void) {
    if(s_coreCartridgeMapper->reset != NULL) {
        s_coreCartridgeMapper->reset();
    }
}

void coreCartridgeCycle(void) {
    if(s_coreCartridgeMapper->cycle != NULL) {
        s_coreCartridgeMapper->cycle();
    }
}

void coreCartridgeCycleDouble(void) {
    if(s_coreCartridgeMapper->cycleDouble != NULL) {
        s_coreCartridgeMapper->cycleDouble();
    }
}

int coreCartridgeSetRom(const void *p_rom, size_t p_size) {
    s_coreCartridgeRomData = p_rom;
    s_coreCartridgeRomSize = p_size;

    if(!coreCartridgeIsRomSizeCorrect()) {
        fprintf(
            stderr,
            "Error: invalid ROM size (expected %d, got %d).\n",
            (int)coreCartridgeComputeRomSize(),
            (int)s_coreCartridgeRomSize
        );

        return 1;
    }

    if(!coreCartridgeIsRamSizeCorrect()) {
        printf(
            "Warning: invalid SRAM size (expected %d, got %d).\n",
            (int)coreCartridgeComputeRamSize(),
            (int)s_coreCartridgeSramSize
        );

        printf("Creating new save data...\n");

        free(s_coreCartridgeSramData);

        s_coreCartridgeSramSize = coreCartridgeComputeRamSize();
        s_coreCartridgeSramData = malloc(s_coreCartridgeSramSize);

        if(s_coreCartridgeSramData == NULL) {
            fprintf(
                stderr,
                "Error: failed to allocate memory for SRAM data.\n"
            );

            return 1;
        }

        memset(s_coreCartridgeSramData, 0, s_coreCartridgeSramSize);
    }

    if(!coreCartridgeIsHeaderChecksumCorrect()) {
        printf(
            "Warning: invalid cartridge header checksum (expected 0x%02x, got 0x%02x).\n",
            coreCartridgeComputeHeaderChecksum(),
            s_coreCartridgeRomData[0x14d]
        );
    }

    if(!coreCartridgeIsGlobalChecksumCorrect()) {
        printf(
            "Warning: invalid cartridge global checksum (expected 0x%04x, got 0x%04x).\n",
            coreCartridgeComputeGlobalChecksum(),
            (s_coreCartridgeRomData[0x14e] << 8) | s_coreCartridgeRomData[0x14f]
        );
    }

    s_coreCartridgeMapper = coreCartridgeDetermineMapper();

    s_coreCartridgeMapper->init(
        s_coreCartridgeRomData,
        s_coreCartridgeRomSize,
        s_coreCartridgeSramData,
        s_coreCartridgeSramSize
    );

    printf("Info: detected cartridge mapper: %s\n", s_coreCartridgeMapper->name);
    printf("Info: ROM size: %d KB.\n", (int)(s_coreCartridgeRomSize >> 10));
    printf("Info: SRAM size: %d KB.\n", (int)(s_coreCartridgeSramSize >> 10));

    return 0;
}

const struct ts_coreCartridgeMapper *coreCartridgeGetMapper(void) {
    return s_coreCartridgeMapper;
}

static const struct ts_coreCartridgeMapper *coreCartridgeDetermineMapper(void) {
    switch(s_coreCartridgeRomData[0x147]) {
        case 0x00: return &g_coreCartridgeMapperNone;
        case 0x01: return &g_coreCartridgeMapperMbc1;
        case 0x02: return &g_coreCartridgeMapperMbc1;
        case 0x03: return &g_coreCartridgeMapperMbc1;
        case 0x0f: return &g_coreCartridgeMapperMbc3;
        case 0x10: return &g_coreCartridgeMapperMbc3;
        case 0x11: return &g_coreCartridgeMapperMbc3;
        case 0x12: return &g_coreCartridgeMapperMbc3;
        case 0x13: return &g_coreCartridgeMapperMbc3;
        case 0x19: return &g_coreCartridgeMapperMbc5;
        case 0x1a: return &g_coreCartridgeMapperMbc5;
        case 0x1b: return &g_coreCartridgeMapperMbc5;
        case 0x1c: return &g_coreCartridgeMapperMbc5;
        case 0x1d: return &g_coreCartridgeMapperMbc5;
        case 0x1e: return &g_coreCartridgeMapperMbc5;
        default: return &g_coreCartridgeMapperInvalid;
    }
}

static uint8_t coreCartridgeComputeHeaderChecksum(void) {
    uint8_t l_checksum = 0;

    for(uint16_t l_address = 0x0134; l_address <= 0x014c; l_address++) {
        l_checksum -= s_coreCartridgeRomData[l_address] + 1;
    }

    return l_checksum;
}

static uint16_t coreCartridgeComputeGlobalChecksum(void) {
    uint16_t l_checksum = 0;

    for(
        size_t l_address = 0;
        l_address < s_coreCartridgeRomSize;
        l_address++
    ) {
        l_checksum += s_coreCartridgeRomData[l_address];
    }

    l_checksum -= s_coreCartridgeRomData[0x014e];
    l_checksum -= s_coreCartridgeRomData[0x014f];

    return l_checksum;
}

static size_t coreCartridgeComputeRomSize(void) {
    switch(s_coreCartridgeRomData[0x148]) {
        case 0x00: return 32768;
        case 0x01: return 65536;
        case 0x02: return 131072;
        case 0x03: return 262144;
        case 0x04: return 524288;
        case 0x05: return 1048576;
        case 0x06: return 2097152;
        case 0x07: return 4194304;
        case 0x08: return 8388608;
        case 0x52: return 1179648;
        case 0x53: return 1310720;
        case 0x54: return 1572864;
        default: return -1;
    }
}

static size_t coreCartridgeComputeRamSize(void) {
    switch(s_coreCartridgeRomData[0x149]) {
        case 0x00: return 0;
        case 0x02: return 8192;
        case 0x03: return 32768;
        case 0x04: return 131072;
        case 0x05: return 65536;
        default: return -1;
    }
}

static bool coreCartridgeIsHeaderChecksumCorrect(void) {
    return coreCartridgeComputeHeaderChecksum()
        == s_coreCartridgeRomData[0x14d];
}

static bool coreCartridgeIsGlobalChecksumCorrect(void) {
    uint16_t l_computedChecksum = coreCartridgeComputeGlobalChecksum();

    return l_computedChecksum == (
        (s_coreCartridgeRomData[0x14e] << 8) | s_coreCartridgeRomData[0x14f]
    );
}

static bool coreCartridgeIsRomSizeCorrect(void) {
    size_t l_computedRomSize = coreCartridgeComputeRomSize();

    if(l_computedRomSize == (size_t)-1) {
        return false;
    }

    return l_computedRomSize == s_coreCartridgeRomSize;
}

static bool coreCartridgeIsRamSizeCorrect(void) {
    size_t l_computedRamSize = coreCartridgeComputeRamSize();

    if(l_computedRamSize == (size_t)-1) {
        return false;
    }

    return l_computedRamSize == s_coreCartridgeSramSize;
}
