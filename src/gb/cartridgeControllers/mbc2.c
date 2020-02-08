#include <stdint.h>

#include <gb/gb.h>

uint8_t gb_cartridge_controller_mbc2_ram_readFunc(uint16_t address) {
	if(GB_CARTRIDGE.controllerData.mbc2.ramg) {
		return GB_CARTRIDGE.saveData[address & 0x01ff] | 0xf0;
	} else {
		return 0xff;
	}
}

uint8_t gb_cartridge_controller_mbc2_rom_readFunc(uint16_t address) {
	if(address <= 0x3fff) {
		return GB_CARTRIDGE.romData[address];
	}

	unsigned int romAddress = (GB_CARTRIDGE.controllerData.mbc2.ramb << 14) | (address & 0x3fff);

	return GB_CARTRIDGE.romData[romAddress % GB_CARTRIDGE.romSize];
}

void gb_cartridge_controller_mbc2_ram_writeFunc(uint16_t address, uint8_t value) {
	if(GB_CARTRIDGE.controllerData.mbc2.ramg) {
		GB_CARTRIDGE.saveData[address & 0x01ff] = value | 0xf0;
	}
}

void gb_cartridge_controller_mbc2_rom_writeFunc(uint16_t address, uint8_t value) {
	if(address <= 0x3fff) {
		if(address & 0x0100) {
			GB_CARTRIDGE.controllerData.mbc2.ramb = value & 0x0f;

			if(GB_CARTRIDGE.controllerData.mbc2.ramb == 0) {
				GB_CARTRIDGE.controllerData.mbc2.ramb = 1;
			}
		} else {
			GB_CARTRIDGE.controllerData.mbc2.ramg = ((value & 0x0f) == 0x0a);
		}
	}
}

void gb_cartridge_controller_mbc2_initFunc() {
	GB_CARTRIDGE.controllerData.mbc2.ramg = false;
	GB_CARTRIDGE.controllerData.mbc2.ramb = 1;
}
