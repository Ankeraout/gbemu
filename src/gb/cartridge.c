#include <stdio.h>
#include <stdlib.h>

#include <gb/gb.h>
#include <gb/cartridgeControllers/none.h>
#include <gb/cartridgeControllers/mbc1.h>
#include <gb/cartridgeControllers/mbc5.h>

const int gb_cartridge_lookupTable_cartridgeType[256] = {
	CARTRIDGE_CONTROLLER_NONE,      // 0x00
	CARTRIDGE_CONTROLLER_MBC1,
	CARTRIDGE_CONTROLLER_MBC1,
	CARTRIDGE_CONTROLLER_MBC1,
	CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_MBC2,
	CARTRIDGE_CONTROLLER_MBC2,
	CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_NONE,
	CARTRIDGE_CONTROLLER_NONE,
	CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_MMM01,
	CARTRIDGE_CONTROLLER_MMM01,
	CARTRIDGE_CONTROLLER_MMM01,
	CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_MBC3,
	CARTRIDGE_CONTROLLER_MBC3,      // 0x10
	CARTRIDGE_CONTROLLER_MBC3,
	CARTRIDGE_CONTROLLER_MBC3,
	CARTRIDGE_CONTROLLER_MBC3,
	CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_MBC5,
	CARTRIDGE_CONTROLLER_MBC5,
	CARTRIDGE_CONTROLLER_MBC5,
	CARTRIDGE_CONTROLLER_MBC5,
	CARTRIDGE_CONTROLLER_MBC5,
	CARTRIDGE_CONTROLLER_MBC5,
	CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_MBC6,      // 0x20
	CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_MBC7,
	CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, // 0x30
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, // 0x40
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, // 0x50
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, // 0x60
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, // 0x70
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, // 0x80
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, // 0x90
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, // 0xA0
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, // 0xB0
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, // 0xC0
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, // 0xD0
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, // 0xE0
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, // 0xF0
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN, CARTRIDGE_CONTROLLER_UNKNOWN,
	CARTRIDGE_CONTROLLER_POCKET_CAMERA,
	CARTRIDGE_CONTROLLER_BANDAI_TAMA5,
	CARTRIDGE_CONTROLLER_HUC3,
	CARTRIDGE_CONTROLLER_HUC1
};

const uint32_t gb_cartridge_lookupTable_romSize[256] = {
	0x00008000, 0x00010000, 0x00020000, 0x00040000, // 0x00
	0x00080000, 0x00100000, 0x00200000, 0x00400000,
	0x00800000, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x10
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x20
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x30
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x40
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0x00120000, 0x00140000, // 0x50
	0x00180000, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x60
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x70
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x80
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x90
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0xA0
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0xB0
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0xC0
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0xD0
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0xE0
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0xF0
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
};

const uint32_t gb_cartridge_lookupTable_ramSize[256] = {
	0x00000000, 0x00000800, 0x00002000, 0x00008000, // 0x00
	0x00020000, 0x00010000, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x10
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x20
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x30
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x40
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x50
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x60
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x70
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x80
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0x90
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0xA0
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0xB0
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0xC0
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0xD0
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0xE0
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 0xF0
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
};

const gb_cartridge_controller_t gb_cartridge_lookupTable_controllerDescriptor[12] = {
	GB_CARTRIDGECONTROLLER_NONE,
	GB_CARTRIDGECONTROLLER_MBC1,
	GB_CARTRIDGECONTROLLER_MBC2,
	{NULL, NULL, NULL, NULL, NULL, false},
	GB_CARTRIDGECONTROLLER_MBC3,
	GB_CARTRIDGECONTROLLER_MBC5,
	{NULL, NULL, NULL, NULL, NULL, false},
	{NULL, NULL, NULL, NULL, NULL, false},
	{NULL, NULL, NULL, NULL, NULL, false},
	{NULL, NULL, NULL, NULL, NULL, false},
	{NULL, NULL, NULL, NULL, NULL, false},
	{NULL, NULL, NULL, NULL, NULL, false}
};

const char *cartridgeControllerName[] = {
	"None",
	"MBC1",
	"MBC2",
	"MMM01",
	"MBC3",
	"MBC5",
	"MBC6",
	"MBC7",
	"Pocket Camera",
	"Bandai TAMA5",
	"HuC3",
	"HuC1",
	"Unknown"
};

const char *cartridgeTypes[] = {
	"ROM ONLY",							// 0x00
	"MBC1",
	"MBC1+RAM",
	"MBC1+RAM+BATTERY",
	"Unknown",
	"MBC2",
	"MBC2+BATTERY",
	"Unknown",
	"ROM+RAM",
	"ROM+RAM+BATTERY",
	"Unknown",
	"MMM01",
	"MMM01+RAM",
	"MMM01+RAM+BATTERY",
	"Unknown",
	"MBC3+TIMER+BATTERY",
	"MBC3+TIMER+RAM+BATTERY",			// 0x10
	"MBC3",
	"MBC3+RAM",
	"MBC3+RAM+BATTERY",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"MBC5",
	"MBC5+RAM",
	"MBC5+RAM+BATTERY",
	"MBC5+RUMBLE",
	"MBC5+RUMBLE+RAM",
	"MBC5+RUMBLE+RAM+BATTERY",
	"Unknown",
	"MBC6",								// 0x20
	"Unknown",
	"MBC7+SENSOR+RUMBLE+RAM+BATTERY",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",							// 0x30
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",							// 0x40
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",							// 0x50
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",							// 0x60
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",							// 0x70
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",							// 0x80
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",							// 0x90
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",							// 0xa0
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",							// 0xb0
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",							// 0xc0
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",							// 0xd0
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",							// 0xe0
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",							// 0xf0
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"POCKET CAMERA",
	"BANDAI TAMA5",
	"HuC3",
	"HuC1+RAM+BATTERY",
};

void gb_cartridge_printCartridgeInfo(uint8_t *romData, size_t romSize) {
	gb_cartridge_header_t *cartridgeHeader = (gb_cartridge_header_t *)&romData[0x100];

	printf("===== CARTRIDGE INFO =====\n");

	printf("Title: ");
	fwrite(cartridgeHeader->title, 1, 11, stdout);

	printf("\nManufacturer code: ");
	fwrite(cartridgeHeader->manufacturerCode, 1, 4, stdout);

	printf("\nCGB flag value: ");

	if(cartridgeHeader->cgbFlag < 0x80) {
		printf("DMG game");
	} else if(cartridgeHeader->cgbFlag == 0x80) {
		printf("DMG game with support for CGB functions");
	} else if(cartridgeHeader->cgbFlag == 0xc0) {
		printf("CGB game (DMG not supported)");
	} else {
		printf("Unknown");
	}

	printf("\nNew licensee code: ");
	fwrite(cartridgeHeader->manufacturerCode, 1, 2, stdout);

	printf("\nSGB flag value: ");

	if(cartridgeHeader->cgbFlag == 0x03) {
		printf("SGB functions supported.");
	} else {
		printf("SGB functions not supported.");
	}

	printf("\nCartridge type: %s\n", cartridgeTypes[cartridgeHeader->cartridgeType]);

	printf("ROM size: ");

	if(gb_cartridge_lookupTable_romSize[cartridgeHeader->romSize] != 0xffffffff) {
		printf("%d kB", gb_cartridge_lookupTable_romSize[cartridgeHeader->romSize] >> 10);
	} else {
		printf("Unknown");
	}

	printf("\nRAM size: "); 
	
	if(gb_cartridge_lookupTable_romSize[cartridgeHeader->ramSize] != 0xffffffff) {
		printf("%d kB", gb_cartridge_lookupTable_ramSize[cartridgeHeader->ramSize] >> 10);
	} else {
		printf("Unknown");
	}

	printf("\nDestination code: ");

	if(cartridgeHeader->destinationCode == 0x00) {
		printf("Japanese");
	} else if(cartridgeHeader->destinationCode == 0x01) {
		printf("Non-japanese");
	} else {
		printf("Unknown");
	}

	if(cartridgeHeader->oldLicenseeCode != 0x33) {
		printf("\nOld licensee code: %02x", cartridgeHeader->oldLicenseeCode);
	}

	printf("\nMask ROM version number: 0x%02x\n", cartridgeHeader->version);

	printf("Header checksum: 0x%02x ", cartridgeHeader->headerChecksum); 
	
	uint8_t headerChecksum = 0;

	for(int i = 0x134; i < 0x14d; i++) {
		headerChecksum -= romData[i] + 1;
	}

	if(headerChecksum == cartridgeHeader->headerChecksum) {
		printf("(correct)");
	} else {
		printf("(incorrect)");
	}

	printf("\nGlobal checksum: 0x%02x%02x ", cartridgeHeader->globalChecksum[0], cartridgeHeader->globalChecksum[1]);

	uint16_t globalChecksum = 0;

	for(size_t i = 0; i < romSize; i++) {
		globalChecksum += romData[i];
	}

	globalChecksum -= romData[0x14e];
	globalChecksum -= romData[0x14f];

	if(globalChecksum == ((cartridgeHeader->globalChecksum[0] << 8) | cartridgeHeader->globalChecksum[1])) {
		printf("(correct)");
	} else {
		printf("(incorrect)");
	}

	printf("\n");
}

int gb_cartridge_init(uint8_t *romData, size_t romSize) {
	gb_cartridge_header_t *cartridgeHeader = (gb_cartridge_header_t *)(&romData[0x100]);

	GB_CARTRIDGE.controller = gb_cartridge_lookupTable_controllerDescriptor[gb_cartridge_lookupTable_cartridgeType[cartridgeHeader->cartridgeType]];
	GB_CARTRIDGE.saveSize = gb_cartridge_lookupTable_ramSize[cartridgeHeader->ramSize];

	if(gbemu_options.printCartridgeInfo) {
		gb_cartridge_printCartridgeInfo(romData, romSize);
	}

	// Check ROM size
	if(romSize != gb_cartridge_lookupTable_romSize[cartridgeHeader->romSize]) {
		fprintf(stderr, "Wrong cartridge ROM size.\n");
		return EXIT_FAILURE;
	}

	// Check cartridge controller type
	if(gb_cartridge_lookupTable_cartridgeType[cartridgeHeader->cartridgeType] == CARTRIDGE_CONTROLLER_UNKNOWN) {
		fprintf(stderr, "Unknown cartridge controller.\n");
		return EXIT_FAILURE;
	}

	// Check RAM size value
	if(gb_cartridge_lookupTable_ramSize[cartridgeHeader->ramSize] == 0xffffffff) {
		fprintf(stderr, "Unknown RAM size.\n");
		return EXIT_FAILURE;
	}

	// If the controller is an MBC2, the save size is forced to be 512 bytes.
	if(gb_cartridge_lookupTable_cartridgeType[cartridgeHeader->cartridgeType] == CARTRIDGE_CONTROLLER_MBC2) {
		GB_CARTRIDGE.saveSize = 512;
	}

	// Allocate memory for save data
	if(GB_CARTRIDGE.saveSize > 0) {
		GB_CARTRIDGE.saveData = malloc(GB_CARTRIDGE.saveSize);

		if(!GB_CARTRIDGE.saveData) {
			fprintf(stderr, "Failed to allocate memory for SRAM.\n");
			return EXIT_FAILURE;
		}
	}

	// Fill in the cartridge structure fields
	GB_CARTRIDGE.romSize = romSize;
	GB_CARTRIDGE.romData = romData;

	if(!GB_CARTRIDGE.controller.supported) {
		fprintf(stderr, "Cartridge controller not supported.\n");
		return EXIT_FAILURE;
	}

	if(GB_CARTRIDGE.controller.initFunc) {
		GB_CARTRIDGE.controller.initFunc();
	}

	return EXIT_SUCCESS;
}
