#ifndef __GB_CARTRIDGECONTROLLERS_MBC3_INCLUDED__
#define __GB_CARTRIDGECONTROLLERS_MBC3_INCLUDED__

#include <stdbool.h>
#include <stdint.h>

#define GB_CARTRIDGECONTROLLER_MBC3 { \
	gb_cartridge_controller_mbc3_rom_readFunc, \
	gb_cartridge_controller_mbc3_rom_writeFunc, \
	gb_cartridge_controller_mbc3_ram_readFunc, \
	gb_cartridge_controller_mbc3_ram_writeFunc, \
	gb_cartridge_controller_mbc3_initFunc, \
	true \
}

typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t daysLow;
	uint8_t daysHigh;
} gb_cartridge_controller_mbc3_time_t;

typedef struct {
	bool protectionDisabled;
	uint8_t romBankNumber;
	uint8_t ramBankNumber;
	bool latchRegister;
	bool halt;
	bool overflow;
	gb_cartridge_controller_mbc3_time_t time;
	gb_cartridge_controller_mbc3_time_t latchedTime;
} gb_cartridge_controller_mbc3_t;

uint8_t gb_cartridge_controller_mbc3_ram_readFunc(uint16_t address);
uint8_t gb_cartridge_controller_mbc3_rom_readFunc(uint16_t address);
void gb_cartridge_controller_mbc3_ram_writeFunc(uint16_t address, uint8_t value);
void gb_cartridge_controller_mbc3_rom_writeFunc(uint16_t address, uint8_t value);
void gb_cartridge_controller_mbc3_initFunc();

#endif
