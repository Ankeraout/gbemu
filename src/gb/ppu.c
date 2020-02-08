#include <time.h>

#include <gb/gb.h>

#define FILL_PALETTE_REGISTER(paletteRegisterName, value) \
	paletteRegisterName.reg = value; \
	\
	paletteRegisterName.colorIndex[3] = value >> 6; \
	paletteRegisterName.colorIndex[2] = (value >> 4) & 0x03; \
	paletteRegisterName.colorIndex[1] = (value >> 2) & 0x03; \
	paletteRegisterName.colorIndex[0] = value & 0x03;

const Uint32 gb_ppu_colors[4] = {
	0xffffffff,
	0xaaaaaaff,
	0x555555ff,
	0x000000ff
};

int gb_ppu_init(SDL_Window *window) {
	srand(time(NULL));

	// Create screen surface
	GB_PPU.screenSurface = SDL_CreateRGBSurface(
		SDL_SWSURFACE,
		GB_SCREEN_WIDTH,
		GB_SCREEN_HEIGHT,
		32,
		0xff000000,
		0x00ff0000,
		0x0000ff00,
		0x000000ff
	);

	if(!GB_PPU.screenSurface) {
		fprintf(stderr, "Failed to create screen surface.\n");
		return EXIT_FAILURE;
	}

	GB_PPU.windowSurface = SDL_GetWindowSurface(window);

	if(!GB_PPU.screenSurface) {
		fprintf(stderr, "Failed to get window surface.\n");
		return EXIT_FAILURE;
	}

	GB_PPU.window = window;

	for(unsigned int i = 0; i < 0x2000; i++) {
		GB_PPU.vram[i] = rand();
	}

	gb_ppu_write_ff47(rand());

	return EXIT_SUCCESS;
}

static inline void gb_ppu_renderScreen() {
	// Draw the screen surface to the window surface
	SDL_BlitScaled(GB_PPU.screenSurface, NULL, GB_PPU.windowSurface, NULL);
}

void gb_ppu_cycle() {
	if(GB_PPU.lcdc_displayEnable) {
		bool statInterruptLine = false;

		if(GB_PPU.currentLine < 144) {
			if(GB_PPU.currentLineCycle == 0) {
				GB_PPU.stat_mode = GB_PPU_STAT_MODE_OAM;

				if(GB_PPU.stat_oamInterruptEnable || (GB_PPU.stat_coincidenceInterruptEnable && (GB_PPU.lyc == GB_PPU.currentLine))) {
					statInterruptLine = true;
				}

				// Perform OAM search all at once as the OAM memory cannot be
				// changed during this phase.
				GB_PPU.oamSearchBufferSize = 0;
				unsigned int oamEntryIndex = 0;

				do {
					gb_ppu_oamEntry_t *entry = (gb_ppu_oamEntry_t *)& GB_PPU.oam[oamEntryIndex << 2];

					if((entry->y > (GB_PPU.lcdc_objSize == 16 ? 0 : 8)) && entry->y < 160) {
						int sprite_y_start = entry->y - 16;
						unsigned int sprite_y_end = sprite_y_start + GB_PPU.lcdc_objSize - 1;

						if((int)GB_PPU.currentLine >= sprite_y_start && GB_PPU.currentLine <= sprite_y_end) {
							// Cache the OAM entry
							if(entry->flags.values.yFlip) {
								GB_PPU.oamSearchBuffer[GB_PPU.oamSearchBufferSize].lineNumber = sprite_y_end - GB_PPU.currentLine;
							} else {
								GB_PPU.oamSearchBuffer[GB_PPU.oamSearchBufferSize].lineNumber = GB_PPU.currentLine - sprite_y_start;
							}

							GB_PPU.oamSearchBuffer[GB_PPU.oamSearchBufferSize++].oamEntry = entry;
						}
					}

					oamEntryIndex++;
				} while((oamEntryIndex < 40) && (GB_PPU.oamSearchBufferSize < 10));

				// Sort sprites by X coordinate (priority condition 1)
				for(int i = 0; i < (int)(GB_PPU.oamSearchBufferSize - 1); i++) {
					for(unsigned int j = i + 1; j < GB_PPU.oamSearchBufferSize; j++) {
						if(GB_PPU.oamSearchBuffer[i].oamEntry->x < GB_PPU.oamSearchBuffer[j].oamEntry->x) {
							gb_ppu_sprite_t tmp = GB_PPU.oamSearchBuffer[i];
							GB_PPU.oamSearchBuffer[i] = GB_PPU.oamSearchBuffer[j];
							GB_PPU.oamSearchBuffer[j] = tmp;
						}
					}
				}

				// Sort sprites by OAM ID (priority condition 2)
				for(int i = 0; i < (int)(GB_PPU.oamSearchBufferSize - 1); i++) {
					for(unsigned int j = i + 1; j < GB_PPU.oamSearchBufferSize; j++) {
						if(GB_PPU.oamSearchBuffer[i].oamEntry->x == GB_PPU.oamSearchBuffer[j].oamEntry->x) {
							if(GB_PPU.oamSearchBuffer[i].oamEntry < GB_PPU.oamSearchBuffer[j].oamEntry) {
								gb_ppu_sprite_t tmp = GB_PPU.oamSearchBuffer[i];
								GB_PPU.oamSearchBuffer[i] = GB_PPU.oamSearchBuffer[j];
								GB_PPU.oamSearchBuffer[j] = tmp;
							}
						}
					}
				}
			} else if(GB_PPU.currentLineCycle == 80) {
				GB_PPU.stat_mode = GB_PPU_STAT_MODE_LCD;
				GB_PPU.drawingX = -GB_PPU.scx;
			} else if(GB_PPU.currentLineCycle == 248) {
				GB_PPU.stat_mode = GB_PPU_STAT_MODE_HBLANK;

				if(GB_PPU.stat_hblankInterruptEnable) {
					statInterruptLine = true;
				}

				if(!gbemu_options.disableGraphics) {
					// Draw background
					if(GB_PPU.lcdc_bgDisplayEnable) {
						uint8_t bg_y = GB_PPU.currentLine + GB_PPU.scy;
						uint8_t bg_x = GB_PPU.scx;
						unsigned int tile_y = (bg_y >> 3) & 0x1f;

						unsigned int tilePixels[8];
						bool refillBuffer = true;

						for(unsigned int x = 0; x < GB_SCREEN_WIDTH; x++) {
							if(refillBuffer) {
								unsigned int tile_x = (bg_x >> 3) & 0x1f;
								uint8_t tileNumber = GB_PPU.vram[GB_PPU.lcdc_bgTileMapDisplaySelect + ((tile_y << 5) | tile_x)] ^ GB_PPU.tileIdInverter;
								unsigned int tileAddress = GB_PPU.lcdc_bgWindowTileDataSelect + ((tileNumber << 4) | ((bg_y & 0x07) << 1));
								uint8_t tileByte0 = GB_PPU.vram[tileAddress];
								uint8_t tileByte1 = GB_PPU.vram[tileAddress | 1];

								for(unsigned int i = 0; i < 8; i++) {
									unsigned int shift = 7 - i;
									unsigned int pixelBit0 = (tileByte0 >> shift) & 0x01;
									unsigned int pixelBit1 = (tileByte1 >> shift) & 0x01;
									tilePixels[i] = (pixelBit1 << 1) | pixelBit0;
								}
							}

							GB_PPU.backgroundBuffer[x] = tilePixels[bg_x & 0x07];

							bg_x++;

							if(!(bg_x & 0x07)) {
								refillBuffer = true;
							}
						}
					}

					// Draw window
					if(GB_PPU.lcdc_bgDisplayEnable && GB_PPU.lcdc_windowDisplayEnable && GB_PPU.currentLine >= GB_PPU.wy) {
						uint8_t wy = GB_PPU.currentLine - GB_PPU.wy;
						uint8_t wx = GB_PPU.wx >= 7 ? 0 : 7 - GB_PPU.wx;
						unsigned int x = GB_PPU.wx >= 7 ? GB_PPU.wx - 7 : 0;
						unsigned int tile_y = (wy >> 3) & 0x1f;
						unsigned int tilePixels[8];
						bool refillBuffer = true;

						for(; x < GB_SCREEN_WIDTH; x++) {
							if(refillBuffer) {
								unsigned int tile_x = (wx >> 3) & 0x1f;
								uint8_t tileNumber = GB_PPU.vram[GB_PPU.lcdc_windowTileMapDisplaySelect + ((tile_y << 5) | tile_x)] ^ GB_PPU.tileIdInverter;
								unsigned int tileAddress = GB_PPU.lcdc_bgWindowTileDataSelect + ((tileNumber << 4) | ((wy & 0x07) << 1));
								uint8_t tileByte0 = GB_PPU.vram[tileAddress];
								uint8_t tileByte1 = GB_PPU.vram[tileAddress | 1];

								for(unsigned int i = 0; i < 8; i++) {
									unsigned int shift = 7 - i;
									unsigned int pixelBit0 = (tileByte0 >> shift) & 0x01;
									unsigned int pixelBit1 = (tileByte1 >> shift) & 0x01;
									tilePixels[i] = (pixelBit1 << 1) | pixelBit0;
								}
							}

							GB_PPU.backgroundBuffer[x] = tilePixels[wx & 0x07];

							wx++;

							if(!(wx & 0x07)) {
								refillBuffer = true;
							}
						}
					}

					// Draw background and window
					for(unsigned int i = 0; i < GB_SCREEN_WIDTH; i++) {
						((Uint32 *)GB_PPU.screenSurface->pixels)[GB_PPU.currentLine * GB_SCREEN_WIDTH + i] = gb_ppu_colors[GB_PPU.bgp.colorIndex[GB_PPU.backgroundBuffer[i]]];
					}

					// TODO: Draw sprites
					if(GB_PPU.lcdc_objDisplayEnable) {
						for(unsigned int i = 0; i < GB_PPU.oamSearchBufferSize; i++) {
							unsigned int lineNumber = GB_PPU.oamSearchBuffer[i].lineNumber;
							gb_ppu_oamEntry_t *oamEntry = GB_PPU.oamSearchBuffer[i].oamEntry;
							unsigned int tileNumber = oamEntry->tileNumber;
							unsigned int x = oamEntry->x - 8;
							unsigned int palette = oamEntry->flags.values.palette;

							if(GB_PPU.lcdc_objSize == 16) {
								tileNumber &= ~0x1;
							}

							unsigned int tileAddress = (tileNumber << 4) | (lineNumber << 1);

							uint8_t tileByte0 = GB_PPU.vram[tileAddress];
							uint8_t tileByte1 = GB_PPU.vram[tileAddress | 1];

							for(unsigned int i = 0; i < 8; i++) {
								unsigned int shift = i;

								if(!oamEntry->flags.values.xFlip) {
									shift = 7 - i;
								}

								unsigned int pixelBit0 = (tileByte0 >> shift) & 0x01;
								unsigned int pixelBit1 = (tileByte1 >> shift) & 0x01;
								unsigned int pixelColor = (pixelBit1 << 1) | pixelBit0;

								if((i + x) < GB_SCREEN_WIDTH + 8) {
									bool drawPixel = pixelColor != 0;

									if(oamEntry->flags.values.renderPriority) {
										if(GB_PPU.backgroundBuffer[i + x]) {
											if(GB_PPU.backgroundBuffer[i + x]) {
												drawPixel = false;
											}
										}
									}

									if(drawPixel) {
										((Uint32 *)GB_PPU.screenSurface->pixels)[GB_PPU.currentLine * GB_SCREEN_WIDTH + i + x] = gb_ppu_colors[GB_PPU.obp[palette].colorIndex[pixelColor]];
									}
								}
							}
						}
					}
				}
			}
		} else if((GB_PPU.currentLine == 144) && (GB_PPU.currentLineCycle == 0)) {
			GB_PPU.stat_mode = GB_PPU_STAT_MODE_VBLANK;
			GB_CPU.ioreg_if |= GB_INT_VBLANK;

			if(GB_PPU.stat_vblankInterruptEnable) {
				statInterruptLine = true;
			}

			GB_PPU.frameFlag = true;

			if(!gbemu_options.disableGraphics) {
				gb_ppu_renderScreen();
			}
		}

		if(GB_PPU.stat_coincidenceInterruptEnable && GB_PPU.currentLine == GB_PPU.lyc) {
			statInterruptLine = true;
		}

		GB_PPU.currentLineCycle += 4;

		if(GB_PPU.currentLineCycle == 456) {
			GB_PPU.currentLineCycle = 0;
			GB_PPU.currentLine++;
			GB_PPU.currentLine %= 154;
		}

		if(statInterruptLine) {
			if(!GB_PPU.statInterruptLine) {
				GB_CPU.ioreg_if |= GB_INT_STAT;
			}
		}

		GB_PPU.statInterruptLine = statInterruptLine;
	} else {
		GB_PPU.currentLine = 0;
		GB_PPU.currentLineCycle = 0;
	}
}

uint8_t gb_ppu_read_vram(uint16_t address) {
	if(GB_PPU.stat_mode != GB_PPU_STAT_MODE_LCD) {
		return GB_PPU.vram[address & 0x1fff];
	} else if(gbemu_options.printVramAccessWarnings) {
		fprintf(stdout, "Warning: Denied VRAM read to 0x%04x\n", address);
	}

	return 0xff;
}

uint8_t gb_ppu_read_oam(uint16_t address) {
	if(!(GB_PPU.stat_mode & 0x02)) {
		address &= 0x00ff;

		if(address >= 0xa0) {
			address &= 0x7f;
		}

		return GB_PPU.oam[address];
	} else if(gbemu_options.printOamAccessWarnings) {
		fprintf(stdout, "Warning: Denied OAM read to 0x%04x\n", address);
	}

	return 0xff;
}

void gb_ppu_write_vram(uint16_t address, uint8_t value) {
	if(GB_PPU.stat_mode != GB_PPU_STAT_MODE_LCD) {
		GB_PPU.vram[address & 0x1fff] = value;
	} else if(gbemu_options.printVramAccessWarnings) {
		fprintf(stdout, "Warning: Denied VRAM write 0x%02x to 0x%04x\n", value, address);
	}
}

void gb_ppu_write_oam(uint16_t address, uint8_t value) {
	if(!(GB_PPU.stat_mode & 0x02)) {
		address &= 0x00ff;

		if(address >= 0xa0) {
			address &= 0x7f;
		}

		GB_PPU.oam[address] = value;
	} else if(gbemu_options.printOamAccessWarnings) {
		fprintf(stdout, "Warning: Denied OAM write 0x%02x to 0x%04x\n", value, address);
	}
}

uint8_t gb_ppu_read_ff40() {
	return GB_PPU.lcdc;
}

uint8_t gb_ppu_read_ff41() {
	uint8_t returnValue = GB_PPU.stat;

	if(GB_PPU.lyc == gb_ppu_read_ff44()) {
		returnValue |= 0x04;
	}

	returnValue |= GB_PPU.stat_mode;

	return returnValue;
}

uint8_t gb_ppu_read_ff42() {
	return GB_PPU.scy;
}

uint8_t gb_ppu_read_ff43() {
	return GB_PPU.scx;
}

uint8_t gb_ppu_read_ff44() {
	return GB_PPU.currentLine;
}

uint8_t gb_ppu_read_ff45() {
	return GB_PPU.lyc;
}

uint8_t gb_ppu_read_ff47() {
	return GB_PPU.bgp.reg;
}

uint8_t gb_ppu_read_ff48() {
	return GB_PPU.obp[0].reg;
}

uint8_t gb_ppu_read_ff49() {
	return GB_PPU.obp[1].reg;
}

uint8_t gb_ppu_read_ff4a() {
	return GB_PPU.wy;
}

uint8_t gb_ppu_read_ff4b() {
	return GB_PPU.wx;
}

void gb_ppu_write_ff40(uint8_t value) {
	GB_PPU.lcdc = value;

	GB_PPU.lcdc_displayEnable = (value & 0x80) != 0;
	GB_PPU.lcdc_windowTileMapDisplaySelect = (value & 0x40) ? 0x1c00 : 0x1800;
	GB_PPU.lcdc_windowDisplayEnable = ((value & 0x20) != 0);
	GB_PPU.lcdc_bgWindowTileDataSelect = (value & 0x10) ? 0x0000 : 0x0800;
	GB_PPU.tileIdInverter = (value & 0x10) ? 0x00 : 0x80;
	GB_PPU.lcdc_bgTileMapDisplaySelect = (value & 0x08) ? 0x1c00 : 0x1800;
	GB_PPU.lcdc_objSize = (value & 0x04) ? 16 : 8;
	GB_PPU.lcdc_objDisplayEnable = ((value & 0x02) != 0);
	GB_PPU.lcdc_bgDisplayEnable = ((value & 0x01) != 0);
}

void gb_ppu_write_ff41(uint8_t value) {
	GB_PPU.stat = (value & 0x78) | 0x80;

	GB_PPU.stat_coincidenceInterruptEnable = ((value & 0x40) != 0);
	GB_PPU.stat_oamInterruptEnable = ((value & 0x20) != 0);
	GB_PPU.stat_vblankInterruptEnable = ((value & 0x10) != 0);
	GB_PPU.stat_hblankInterruptEnable = ((value & 0x08) != 0);
}

void gb_ppu_write_ff42(uint8_t value) {
	GB_PPU.scy = value;
}

void gb_ppu_write_ff43(uint8_t value) {
	GB_PPU.scx = value;
}

void gb_ppu_write_ff45(uint8_t value) {
	GB_PPU.lyc = value;
}

void gb_ppu_write_ff47(uint8_t value) {
	FILL_PALETTE_REGISTER(GB_PPU.bgp, value);
}

void gb_ppu_write_ff48(uint8_t value) {
	FILL_PALETTE_REGISTER(GB_PPU.obp[0], value);
}

void gb_ppu_write_ff49(uint8_t value) {
	FILL_PALETTE_REGISTER(GB_PPU.obp[1], value);
}

void gb_ppu_write_ff4a(uint8_t value) {
	GB_PPU.wy = value;
}

void gb_ppu_write_ff4b(uint8_t value) {
	GB_PPU.wx = value;
}
