#ifndef __GB_CARTRIDGECONTROLLERS_MBC1_INCLUDED__
#define __GB_CARTRIDGECONTROLLERS_MBC1_INCLUDED__

#include <stdbool.h>
#include <stdint.h>

#define GB_CARTRIDGECONTROLLER_MBC1 { \
	gb_cartridge_controller_mbc1_rom_readFunc, \
	gb_cartridge_controller_mbc1_rom_writeFunc, \
	gb_cartridge_controller_mbc1_ram_readFunc, \
	gb_cartridge_controller_mbc1_ram_writeFunc, \
	gb_cartridge_controller_mbc1_initFunc, \
	true \
}

typedef struct {
	bool ramBankingMode;
	bool ramEnabled;
	unsigned int bankNumber1;
	unsigned int bankNumber2;
} gb_cartridge_controller_mbc1_t;

uint8_t gb_cartridge_controller_mbc1_ram_readFunc(uint16_t address);
uint8_t gb_cartridge_controller_mbc1_rom_readFunc(uint16_t address);
void gb_cartridge_controller_mbc1_ram_writeFunc(uint16_t address, uint8_t value);
void gb_cartridge_controller_mbc1_rom_writeFunc(uint16_t address, uint8_t value);
void gb_cartridge_controller_mbc1_initFunc();

#endif
