#include <gbemu.h>

#include <gb/cartridgeControllers/none.h>
#include <gb/gb.h>

uint8_t gb_cartridge_controller_none_ram_readFunc(uint16_t address) {
	address &= 0x1fff;

	if(address >= GB_CARTRIDGE.saveSize) {
		return 0xff;
	}

	return GB_CARTRIDGE.saveData[address];
}

uint8_t gb_cartridge_controller_none_rom_readFunc(uint16_t address) {
	address &= 0x7fff;

	if(address >= GB_CARTRIDGE.romSize) {
		return 0xff;
	}

	return GB_CARTRIDGE.romData[address];
}

void gb_cartridge_controller_none_ram_writeFunc(uint16_t address, uint8_t value) {
	address &= 0x1fff;

	if(address < GB_CARTRIDGE.saveSize) {
		GB_CARTRIDGE.saveData[address] = value;
	}
}

void gb_cartridge_controller_none_rom_writeFunc(uint16_t address, uint8_t value) {
	// Nothing to do
	UNUSED_PARAMETER(address);
	UNUSED_PARAMETER(value);
}
