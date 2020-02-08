#include <stdint.h>

#include <gb/gb.h>

uint8_t gb_cartridge_controller_mbc3_ram_readFunc(uint16_t address) {
	switch(GB_CARTRIDGE.controllerData.mbc3.ramBankNumber) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			{
				unsigned int ramAddress = (GB_CARTRIDGE.controllerData.mbc3.ramBankNumber << 13) | (address & 0x1fff);
				
				if(GB_CARTRIDGE.saveSize) {
					return GB_CARTRIDGE.saveData[ramAddress % GB_CARTRIDGE.saveSize];
				}
			}

			return 0xff;

		case 0x08:
			if(GB_CARTRIDGE.controllerData.mbc3.latchRegister) {
				return GB_CARTRIDGE.controllerData.mbc3.time.seconds;
			} else {
				return GB_CARTRIDGE.controllerData.mbc3.latchedTime.seconds;
			}

		case 0x09:
			if(GB_CARTRIDGE.controllerData.mbc3.latchRegister) {
				return GB_CARTRIDGE.controllerData.mbc3.time.minutes;
			} else {
				return GB_CARTRIDGE.controllerData.mbc3.latchedTime.minutes;
			}

		case 0x0a:
			if(GB_CARTRIDGE.controllerData.mbc3.latchRegister) {
				return GB_CARTRIDGE.controllerData.mbc3.time.hours;
			} else {
				return GB_CARTRIDGE.controllerData.mbc3.latchedTime.hours;
			}

		case 0x0b:
			if(GB_CARTRIDGE.controllerData.mbc3.latchRegister) {
				return GB_CARTRIDGE.controllerData.mbc3.time.daysLow;
			} else {
				return GB_CARTRIDGE.controllerData.mbc3.latchedTime.daysLow;
			}

		case 0x0c:
			{
				uint8_t registerValue = 0x00;

				if(GB_CARTRIDGE.controllerData.mbc3.overflow) {
					registerValue |= 0x80;
				}

				if(GB_CARTRIDGE.controllerData.mbc3.halt) {
					registerValue |= 0x40;
				}

				if(GB_CARTRIDGE.controllerData.mbc3.latchRegister) {
					registerValue |= GB_CARTRIDGE.controllerData.mbc3.time.daysHigh & 0x01;
				} else {
					registerValue |= GB_CARTRIDGE.controllerData.mbc3.latchedTime.daysHigh & 0x01;
				}

				return registerValue;
			}

		default:
			return 0xff;
	}
}

uint8_t gb_cartridge_controller_mbc3_rom_readFunc(uint16_t address) {
	if(address & 0x4000) {
		unsigned int romAddress = (address & 0x3fff) | (GB_CARTRIDGE.controllerData.mbc3.romBankNumber << 14);

		return GB_CARTRIDGE.romData[romAddress % GB_CARTRIDGE.romSize];
	} else {
		return GB_CARTRIDGE.romData[address];
	}
}

void gb_cartridge_controller_mbc3_ram_writeFunc(uint16_t address, uint8_t value) {
	switch(GB_CARTRIDGE.controllerData.mbc3.ramBankNumber) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			{
				unsigned int ramAddress = (GB_CARTRIDGE.controllerData.mbc3.ramBankNumber << 13) | (address & 0x1fff);
				
				if(GB_CARTRIDGE.saveSize) {
					GB_CARTRIDGE.saveData[ramAddress % GB_CARTRIDGE.saveSize] = value;
				}
			}

			break;

		case 0x08:
			GB_CARTRIDGE.controllerData.mbc3.time.seconds = value;
			break;

		case 0x09:
			GB_CARTRIDGE.controllerData.mbc3.time.minutes = value;
			break;

		case 0x0a:
			GB_CARTRIDGE.controllerData.mbc3.time.hours = value;
			break;

		case 0x0b:
			GB_CARTRIDGE.controllerData.mbc3.time.daysLow = value;
			break;

		case 0x0c:
			GB_CARTRIDGE.controllerData.mbc3.time.daysHigh = value & 0x01;
			GB_CARTRIDGE.controllerData.mbc3.halt = (value >> 6) & 0x01;
			GB_CARTRIDGE.controllerData.mbc3.overflow = value >> 7;
			break;

		default:
			break;
	}
}

void gb_cartridge_controller_mbc3_rom_writeFunc(uint16_t address, uint8_t value) {
	switch(address & 0x6000) {
		case 0x0000:
			GB_CARTRIDGE.controllerData.mbc3.protectionDisabled = (value == 0x0a);
			break;
		
		case 0x2000:
			GB_CARTRIDGE.controllerData.mbc3.romBankNumber = value & 0x7f;
			break;
		
		case 0x4000:
			GB_CARTRIDGE.controllerData.mbc3.ramBankNumber = value & 0x0f;
			break;
		
		case 0x6000:
			GB_CARTRIDGE.controllerData.mbc3.latchRegister = value & 0x01;
			break;
	}
}

void gb_cartridge_controller_mbc3_initFunc() {
	memset(&GB_CARTRIDGE.controllerData.mbc3, 0, sizeof(GB_CARTRIDGE.controllerData.mbc3));
}
