#ifndef __GB_CARTRIDGECONTROLLERS_NONE_INCLUDED__
#define __GB_CARTRIDGECONTROLLERS_NONE_INCLUDED__

#include <stdbool.h>
#include <stdint.h>

#define GB_CARTRIDGECONTROLLER_NONE { \
	gb_cartridge_controller_none_rom_readFunc, \
	gb_cartridge_controller_none_rom_writeFunc, \
	gb_cartridge_controller_none_ram_readFunc, \
	gb_cartridge_controller_none_ram_writeFunc, \
	NULL, \
	true \
}

uint8_t gb_cartridge_controller_none_ram_readFunc(uint16_t address);
uint8_t gb_cartridge_controller_none_rom_readFunc(uint16_t address);
void gb_cartridge_controller_none_ram_writeFunc(uint16_t address, uint8_t value);
void gb_cartridge_controller_none_rom_writeFunc(uint16_t address, uint8_t value);

#endif
