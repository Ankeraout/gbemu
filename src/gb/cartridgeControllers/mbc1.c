#include <stdint.h>

#include <gb/gb.h>

uint8_t gb_cartridge_controller_mbc1_ram_readFunc(uint16_t address) {
	if(!GB_CARTRIDGE.controllerData.mbc1.ramEnabled) {
		return 0xff;
	}

	if(GB_CARTRIDGE.saveSize) {
		address &= 0x1fff;

		if(GB_CARTRIDGE.controllerData.mbc1.ramBankingMode) {
			address |= GB_CARTRIDGE.controllerData.mbc1.bankNumber2 << 13;
		}

		return GB_CARTRIDGE.saveData[address % GB_CARTRIDGE.saveSize];
	}

	return 0xff;
}

uint8_t gb_cartridge_controller_mbc1_rom_readFunc(uint16_t address) {
	if(address <= 0x3fff) {
		if(GB_CARTRIDGE.controllerData.mbc1.ramBankingMode) {
			unsigned int romAddress = address | (GB_CARTRIDGE.controllerData.mbc1.bankNumber2 << 19);

			return GB_CARTRIDGE.romData[romAddress % GB_CARTRIDGE.romSize];
		} else {
			return GB_CARTRIDGE.romData[address];
		}
	}

	address &= 0x3fff;

	unsigned int bankNumber = GB_CARTRIDGE.controllerData.mbc1.bankNumber1;

	bankNumber |= GB_CARTRIDGE.controllerData.mbc1.bankNumber2 << 5;

	unsigned int romAddress = address | (bankNumber << 14);
	
	return GB_CARTRIDGE.romData[romAddress % GB_CARTRIDGE.romSize];
}

void gb_cartridge_controller_mbc1_ram_writeFunc(uint16_t address, uint8_t value) {
	if(!GB_CARTRIDGE.controllerData.mbc1.ramEnabled) {
		return;
	}

	if(GB_CARTRIDGE.saveSize) {
		address &= 0x1fff;

		if(GB_CARTRIDGE.controllerData.mbc1.ramBankingMode) {
			address |= GB_CARTRIDGE.controllerData.mbc1.bankNumber2 << 13;
		}

		GB_CARTRIDGE.saveData[address % GB_CARTRIDGE.saveSize] = value;
	}
}

void gb_cartridge_controller_mbc1_rom_writeFunc(uint16_t address, uint8_t value) {
	switch(address & 0x6000) {
		case 0x0000:
			GB_CARTRIDGE.controllerData.mbc1.ramEnabled = (value & 0x0f) == 0x0a;
			break;

		case 0x2000:
			GB_CARTRIDGE.controllerData.mbc1.bankNumber1 = value & 0x1f;

			if(GB_CARTRIDGE.controllerData.mbc1.bankNumber1 == 0) {
				GB_CARTRIDGE.controllerData.mbc1.bankNumber1 = 1;
			}

			break;

		case 0x4000:
			GB_CARTRIDGE.controllerData.mbc1.bankNumber2 = value & 0x03;
			break;

		case 0x6000:
			GB_CARTRIDGE.controllerData.mbc1.ramBankingMode = value & 0x01;
			break;
	}
}

void gb_cartridge_controller_mbc1_initFunc() {
	GB_CARTRIDGE.controllerData.mbc1.bankNumber1 = 1;
	GB_CARTRIDGE.controllerData.mbc1.bankNumber2 = 0;
	GB_CARTRIDGE.controllerData.mbc1.ramBankingMode = false;
	GB_CARTRIDGE.controllerData.mbc1.ramEnabled = false;
}
