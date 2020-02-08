#ifndef __GB_GB_H_INCLUDED__
#define __GB_GB_H_INCLUDED__

#include <stddef.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#define GB gb

typedef uint8_t gb_io_readfunc_t(uint16_t address);
typedef void gb_io_writefunc_t(uint16_t address, uint8_t value);
typedef uint8_t gb_ioreg_readfunc_t();
typedef void gb_ioreg_writefunc_t(uint8_t value);

void gb_cycleNoCPU();
int gb_init(SDL_Window *window, uint8_t *biosData, uint8_t *romData, size_t romSize);
void gb_frame();
int gb_loadSaveFile(const char *saveFileName);
int gb_saveSaveFile(const char *saveFileName);

#define GB_RAM_SIZE 0x2000
#define GB_HRAM_SIZE 0x80

#include <gb/apu.h>
#include <gb/bios.h>
#include <gb/cartridge.h>
#include <gb/cpu.h>
#include <gb/dma.h>
#include <gb/joypad.h>
#include <gb/mmu.h>
#include <gb/ppu.h>
#include <gb/serial.h>
#include <gb/timer.h>

typedef struct {
	uint8_t hram[GB_HRAM_SIZE];
	uint8_t ram[GB_RAM_SIZE];

	gb_apu_t apu;
	gb_bios_t bios;
	gb_cartridge_t cartridge;
	gb_cpu_t cpu;
	gb_dma_t dma;
	gb_joypad_t joypad;
	gb_ppu_t ppu;
	gb_serial_t serial;
	gb_timer_t timer;

	unsigned int clocks;
} gb_t;

extern gb_t GB;

#endif
