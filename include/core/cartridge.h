#ifndef __INCLUDE_CORE_CARTRIDGE_H__
#define __INCLUDE_CORE_CARTRIDGE_H__

#include <stdint.h>

void coreCartridgeReset(void);
int coreCartridgeSetRom(const void *p_rom, size_t p_size);
uint8_t coreCartridgeReadRom(uint16_t p_address);
void coreCartridgeWriteRom(uint16_t p_address, uint8_t p_value);
uint8_t coreCartridgeReadSram(uint16_t p_address);
void coreCartridgeWriteSram(uint16_t p_address, uint8_t p_value);

#endif
