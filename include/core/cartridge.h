#ifndef __INCLUDE_CORE_CARTRIDGE_H__
#define __INCLUDE_CORE_CARTRIDGE_H__

#include <stddef.h>
#include <stdint.h>

#include "core/mappers/none.h"

typedef int (*t_coreCartridgeMapperInitFunc)(
    const uint8_t *p_romData,
    size_t p_romSize,
    uint8_t *p_ramData,
    size_t p_ramSize
);
typedef void (*t_coreCartridgeMapperResetFunc)(void);
typedef uint8_t (*t_coreCartridgeMapperReadFunc)(uint16_t p_address);
typedef void (*t_coreCartridgeMapperWriteFunc)(
    uint16_t p_address,
    uint8_t p_value
);

struct ts_coreCartridgeMapper {
    char *name;
    t_coreCartridgeMapperInitFunc init;
    t_coreCartridgeMapperResetFunc reset;
    t_coreCartridgeMapperReadFunc readRom;
    t_coreCartridgeMapperReadFunc readRam;
    t_coreCartridgeMapperWriteFunc writeRom;
    t_coreCartridgeMapperWriteFunc writeRam;
};

void coreCartridgeReset(void);
int coreCartridgeSetRom(const void *p_rom, size_t p_size);
const struct ts_coreCartridgeMapper *coreCartridgeGetMapper(void);

#endif
