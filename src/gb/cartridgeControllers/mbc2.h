#ifndef __GB_CARTRIDGECONTROLLERS_MBC2_INCLUDED__
#define __GB_CARTRIDGECONTROLLERS_MBC2_INCLUDED__

#include <stdbool.h>
#include <stdint.h>

#define GB_CARTRIDGECONTROLLER_MBC2 { \
	gb_cartridge_controller_mbc2_rom_readFunc, \
	gb_cartridge_controller_mbc2_rom_writeFunc, \
	gb_cartridge_controller_mbc2_ram_readFunc, \
	gb_cartridge_controller_mbc2_ram_writeFunc, \
	gb_cartridge_controller_mbc2_initFunc, \
	true \
}

typedef struct {
	bool ramg;
	unsigned int ramb;
} gb_cartridge_controller_mbc2_t;

uint8_t gb_cartridge_controller_mbc2_ram_readFunc(uint16_t address);
uint8_t gb_cartridge_controller_mbc2_rom_readFunc(uint16_t address);
void gb_cartridge_controller_mbc2_ram_writeFunc(uint16_t address, uint8_t value);
void gb_cartridge_controller_mbc2_rom_writeFunc(uint16_t address, uint8_t value);
void gb_cartridge_controller_mbc2_initFunc();

#endif
