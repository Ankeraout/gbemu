#ifndef __GB_PPU_H_INCLUDED__
#define __GB_PPU_H_INCLUDED__

#include <gb/gb.h>

#include <SDL2/SDL.h>

#define GB_PPU (GB.ppu)

#define GB_VRAM_SIZE 0x2000
#define GB_OAM_SIZE 0xa0

#define GB_SCREEN_WIDTH 160
#define GB_SCREEN_HEIGHT 144

/*
OAM entry structure:
	byte 0: y location
	byte 1: x location minus 8
	byte 2: tile number
	byte 3: flags
*/

/*
Flags structure:
	bit 7: render priority
	bit 6: y flip
	bit 5: x flip
	bit 4: palette number
*/

typedef enum {
	GB_PPU_STAT_MODE_HBLANK,
	GB_PPU_STAT_MODE_VBLANK,
	GB_PPU_STAT_MODE_OAM,
	GB_PPU_STAT_MODE_LCD
} gb_ppu_statMode_t;

typedef PACKED_STRUCT ({
	uint8_t y;
	uint8_t x;
	uint8_t tileNumber;

	union {
		uint8_t value;
	
		PACKED_STRUCT({
			uint8_t data : 4;
			uint8_t palette : 1;
			uint8_t xFlip : 1;
			uint8_t yFlip : 1;
			uint8_t renderPriority : 1;
		}) values;
	} flags;
}) gb_ppu_oamEntry_t;

typedef struct {
	unsigned int colorIndex[4];
	uint8_t reg;
} gb_ppu_palette_t;

typedef struct {
	unsigned colorValue;
	gb_ppu_palette_t *sourcePalette;
} gb_ppu_pixelFifoEntry_t;

typedef struct {
	gb_ppu_oamEntry_t *oamEntry;
	unsigned int lineNumber;
} gb_ppu_sprite_t;

typedef struct {
	uint8_t lcdc;
	bool lcdc_displayEnable;
	uint16_t lcdc_windowTileMapDisplaySelect;
	bool lcdc_windowDisplayEnable;
	uint16_t lcdc_bgWindowTileDataSelect;
	uint16_t lcdc_bgTileMapDisplaySelect;
	unsigned int lcdc_objSize;
	bool lcdc_objDisplayEnable;
	bool lcdc_bgDisplayEnable;

	uint8_t stat;
	bool stat_coincidenceInterruptEnable;
	bool stat_oamInterruptEnable;
	bool stat_vblankInterruptEnable;
	bool stat_hblankInterruptEnable;
	gb_ppu_statMode_t stat_mode;

	uint8_t scx;
	uint8_t scy;
	uint8_t lyc;
	uint8_t wx;
	uint8_t wy;
	gb_ppu_palette_t bgp;
	gb_ppu_palette_t obp[2];
	uint8_t vram[GB_VRAM_SIZE];
	uint8_t oam[GB_OAM_SIZE];
	uint8_t tileIdInverter;

	SDL_Surface *screenSurface;
	SDL_Surface *windowSurface;
	SDL_Window *window;

	unsigned int currentLine;
	unsigned int currentLineCycle;
	int drawingX;
	bool statInterruptLine;

	gb_ppu_sprite_t oamSearchBuffer[10];
	unsigned int oamSearchBufferSize;

	bool frameFlag;

	unsigned int backgroundBuffer[GB_SCREEN_WIDTH];
} gb_ppu_t;

int gb_ppu_init(SDL_Window *window);
void gb_ppu_cycle();

uint8_t gb_ppu_read_vram(uint16_t address);
uint8_t gb_ppu_read_oam(uint16_t address);

uint8_t gb_ppu_read_ff40();
uint8_t gb_ppu_read_ff41();
uint8_t gb_ppu_read_ff42();
uint8_t gb_ppu_read_ff43();
uint8_t gb_ppu_read_ff44();
uint8_t gb_ppu_read_ff45();
uint8_t gb_ppu_read_ff47();
uint8_t gb_ppu_read_ff48();
uint8_t gb_ppu_read_ff49();
uint8_t gb_ppu_read_ff4a();
uint8_t gb_ppu_read_ff4b();

void gb_ppu_write_vram(uint16_t address, uint8_t value);
void gb_ppu_write_oam(uint16_t address, uint8_t value);

void gb_ppu_write_ff40(uint8_t value);
void gb_ppu_write_ff41(uint8_t value);
void gb_ppu_write_ff42(uint8_t value);
void gb_ppu_write_ff43(uint8_t value);
void gb_ppu_write_ff45(uint8_t value);
void gb_ppu_write_ff47(uint8_t value);
void gb_ppu_write_ff48(uint8_t value);
void gb_ppu_write_ff49(uint8_t value);
void gb_ppu_write_ff4a(uint8_t value);
void gb_ppu_write_ff4b(uint8_t value);

#endif
