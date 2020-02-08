#ifndef __GB_CARTRIDGE_H_INCLUDED__
#define __GB_CARTRIDGE_H_INCLUDED__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <gbemu.h>
#include <gb/gb.h>
#include <gb/cartridgeControllers/mbc1.h>
#include <gb/cartridgeControllers/mbc2.h>
#include <gb/cartridgeControllers/mbc3.h>
#include <gb/cartridgeControllers/mbc5.h>

#define gb_cartridge_rom_read(address) GB_CARTRIDGE.controller.rom_readFunc(address)
#define gb_cartridge_rom_write(address, value) GB_CARTRIDGE.controller.rom_writeFunc(address, value)
#define gb_cartridge_ram_read(address) GB_CARTRIDGE.controller.ram_readFunc(address)
#define gb_cartridge_ram_write(address, value) GB_CARTRIDGE.controller.ram_writeFunc(address, value)

#define GB_CARTRIDGE (GB.cartridge)

typedef void gb_cartridge_controller_initFunc_t();

enum {
	CARTRIDGE_CONTROLLER_NONE,
	CARTRIDGE_CONTROLLER_MBC1,
	CARTRIDGE_CONTROLLER_MBC2,
	CARTRIDGE_CONTROLLER_MMM01,
	CARTRIDGE_CONTROLLER_MBC3,
	CARTRIDGE_CONTROLLER_MBC5,
	CARTRIDGE_CONTROLLER_MBC6,
	CARTRIDGE_CONTROLLER_MBC7,
	CARTRIDGE_CONTROLLER_POCKET_CAMERA,
	CARTRIDGE_CONTROLLER_BANDAI_TAMA5,
	CARTRIDGE_CONTROLLER_HUC3,
	CARTRIDGE_CONTROLLER_HUC1,
	CARTRIDGE_CONTROLLER_UNKNOWN
};

typedef struct {
	gb_io_readfunc_t *rom_readFunc;
	gb_io_writefunc_t *rom_writeFunc;
	gb_io_readfunc_t *ram_readFunc;
	gb_io_writefunc_t *ram_writeFunc;
	gb_cartridge_controller_initFunc_t *initFunc;
	bool supported;
} gb_cartridge_controller_t;

typedef struct {
	uint8_t *romData;
	size_t romSize;
	uint8_t *saveData;
	size_t saveSize;
	gb_cartridge_controller_t controller;

	union {
		gb_cartridge_controller_mbc1_t mbc1;
		gb_cartridge_controller_mbc2_t mbc2;
		gb_cartridge_controller_mbc3_t mbc3;
		gb_cartridge_controller_mbc5_t mbc5;
	} controllerData;
} gb_cartridge_t;

typedef PACKED_STRUCT(
	{
		uint8_t entryPoint[4];
		uint8_t nintendoLogo[0x30];
		uint8_t title[11];
		uint8_t manufacturerCode[4];
		uint8_t cgbFlag;
		uint8_t newLicenseeCode[2];
		uint8_t sgbFlag;
		uint8_t cartridgeType;
		uint8_t romSize;
		uint8_t ramSize;
		uint8_t destinationCode;
		uint8_t oldLicenseeCode;
		uint8_t version;
		uint8_t headerChecksum;
		uint8_t globalChecksum[2];
	}
) gb_cartridge_header_t;

int gb_cartridge_init(uint8_t *romData, size_t romSize);

#endif
