#include <stdint.h>

#include <gb/gb.h>

uint8_t gb_cartridge_controller_mbc5_ram_readFunc(uint16_t address) {
	if(GB_CARTRIDGE.saveSize) {
		address &= 0x1fff;

		if(GB_CARTRIDGE.controllerData.mbc5.ramEnabled) {
			unsigned int ramAddress = (GB_CARTRIDGE.controllerData.mbc5.ramBank << 13) | address;
			
			return GB_CARTRIDGE.saveData[ramAddress % GB_CARTRIDGE.saveSize];
		}
	}

	return 0xff;
}

uint8_t gb_cartridge_controller_mbc5_rom_readFunc(uint16_t address) {
	if(address <= 0x3fff) {
		return GB_CARTRIDGE.romData[address];
	}

	address &= 0x3fff;

	unsigned int romAddress = (GB_CARTRIDGE.controllerData.mbc5.romBank << 14) | address;

	return GB_CARTRIDGE.romData[romAddress % GB_CARTRIDGE.romSize];
}

void gb_cartridge_controller_mbc5_ram_writeFunc(uint16_t address, uint8_t value) {
	if(GB_CARTRIDGE.saveSize) {
		address &= 0x1fff;

		if(GB_CARTRIDGE.controllerData.mbc5.ramEnabled) {
			unsigned int ramAddress = (GB_CARTRIDGE.controllerData.mbc5.ramBank << 13) | address;

			GB_CARTRIDGE.saveData[ramAddress % GB_CARTRIDGE.saveSize] = value;
		}
	}
}

void gb_cartridge_controller_mbc5_rom_writeFunc(uint16_t address, uint8_t value) {
	switch(address & 0x7000) {
		case 0x0000:
		case 0x1000:
			GB_CARTRIDGE.controllerData.mbc5.ramEnabled = value == 0x0a;
			break;

		case 0x2000:
			GB_CARTRIDGE.controllerData.mbc5.romBank &= 0x100;
			GB_CARTRIDGE.controllerData.mbc5.romBank |= value;
			break;

		case 0x3000:
			GB_CARTRIDGE.controllerData.mbc5.romBank &= 0x0ff;
			GB_CARTRIDGE.controllerData.mbc5.romBank |= (value & 0x01) << 8;
			break;

		case 0x4000:
		case 0x5000:
			GB_CARTRIDGE.controllerData.mbc5.ramBank = value & 0x0f;
			break;

		default:
			// Ignore
			break;
	}
}

void gb_cartridge_controller_mbc5_initFunc() {
	GB_CARTRIDGE.controllerData.mbc5.ramEnabled = false;
	GB_CARTRIDGE.controllerData.mbc5.romBank = 1;
	GB_CARTRIDGE.controllerData.mbc5.ramBank = 0;
}
