#ifndef __GB_CARTRIDGECONTROLLERS_MBC5_INCLUDED__
#define __GB_CARTRIDGECONTROLLERS_MBC5_INCLUDED__

#include <stdbool.h>
#include <stdint.h>

#define GB_CARTRIDGECONTROLLER_MBC5 { \
	gb_cartridge_controller_mbc5_rom_readFunc, \
	gb_cartridge_controller_mbc5_rom_writeFunc, \
	gb_cartridge_controller_mbc5_ram_readFunc, \
	gb_cartridge_controller_mbc5_ram_writeFunc, \
	gb_cartridge_controller_mbc5_initFunc, \
	true \
}

typedef struct {
	bool ramEnabled;
	unsigned int romBank;
	unsigned int ramBank;
} gb_cartridge_controller_mbc5_t;

uint8_t gb_cartridge_controller_mbc5_ram_readFunc(uint16_t address);
uint8_t gb_cartridge_controller_mbc5_rom_readFunc(uint16_t address);
void gb_cartridge_controller_mbc5_ram_writeFunc(uint16_t address, uint8_t value);
void gb_cartridge_controller_mbc5_rom_writeFunc(uint16_t address, uint8_t value);
void gb_cartridge_controller_mbc5_initFunc();

#endif
