#include <gb/gb.h>

static gb_ioreg_readfunc_t *const gb_mmu_ioreg_readFuncs[0x80] = {
	// 0xff00
	gb_joypad_read_ff00,
	gb_serial_read_ff01,
	gb_serial_read_ff02,
	NULL,
	gb_timer_read_ff04,
	gb_timer_read_ff05,
	gb_timer_read_ff06,
	gb_timer_read_ff07,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	gb_cpu_read_ff0f,

	// 0xff10
	gb_apu_read_ff10,
	gb_apu_read_ff11,
	gb_apu_read_ff12,
	NULL,
	gb_apu_read_ff14,
	NULL,
	gb_apu_read_ff16,
	gb_apu_read_ff17,
	NULL,
	gb_apu_read_ff19,
	gb_apu_read_ff1a,
	NULL,
	gb_apu_read_ff1c,
	NULL,
	gb_apu_read_ff1e,
	NULL,
	
	// 0xff20
	gb_apu_read_ff20,
	gb_apu_read_ff21,
	gb_apu_read_ff22,
	gb_apu_read_ff23,
	gb_apu_read_ff24,
	gb_apu_read_ff25,
	gb_apu_read_ff26,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	
	// 0xff30
	gb_apu_read_ff30,
	gb_apu_read_ff31,
	gb_apu_read_ff32,
	gb_apu_read_ff33,
	gb_apu_read_ff34,
	gb_apu_read_ff35,
	gb_apu_read_ff36,
	gb_apu_read_ff37,
	gb_apu_read_ff38,
	gb_apu_read_ff39,
	gb_apu_read_ff3a,
	gb_apu_read_ff3b,
	gb_apu_read_ff3c,
	gb_apu_read_ff3d,
	gb_apu_read_ff3e,
	gb_apu_read_ff3f,
	
	// 0xff40
	gb_ppu_read_ff40,
	gb_ppu_read_ff41,
	gb_ppu_read_ff42,
	gb_ppu_read_ff43,
	gb_ppu_read_ff44,
	gb_ppu_read_ff45,
	gb_dma_read_ff46,
	gb_ppu_read_ff47,
	gb_ppu_read_ff48,
	gb_ppu_read_ff49,
	gb_ppu_read_ff4a,
	gb_ppu_read_ff4b,
	NULL,
	NULL,
	NULL,
	NULL,
	
	// 0xff50
	gb_bios_read_ff50,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	
	// 0xff60
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	
	// 0xff70
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static gb_ioreg_writefunc_t *const gb_mmu_ioreg_writeFuncs[0x80] = {
	// 0xff00
	gb_joypad_write_ff00,
	gb_serial_write_ff01,
	gb_serial_write_ff02,
	NULL,
	gb_timer_write_ff04,
	gb_timer_write_ff05,
	gb_timer_write_ff06,
	gb_timer_write_ff07,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	gb_cpu_write_ff0f,

	// 0xff10
	gb_apu_write_ff10,
	gb_apu_write_ff11,
	gb_apu_write_ff12,
	gb_apu_write_ff13,
	gb_apu_write_ff14,
	NULL,
	gb_apu_write_ff16,
	gb_apu_write_ff17,
	gb_apu_write_ff18,
	gb_apu_write_ff19,
	gb_apu_write_ff1a,
	gb_apu_write_ff1b,
	gb_apu_write_ff1c,
	gb_apu_write_ff1d,
	gb_apu_write_ff1e,
	NULL,
	
	// 0xff20
	gb_apu_write_ff20,
	gb_apu_write_ff21,
	gb_apu_write_ff22,
	gb_apu_write_ff23,
	gb_apu_write_ff24,
	gb_apu_write_ff25,
	gb_apu_write_ff26,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	
	// 0xff30
	gb_apu_write_ff30,
	gb_apu_write_ff31,
	gb_apu_write_ff32,
	gb_apu_write_ff33,
	gb_apu_write_ff34,
	gb_apu_write_ff35,
	gb_apu_write_ff36,
	gb_apu_write_ff37,
	gb_apu_write_ff38,
	gb_apu_write_ff39,
	gb_apu_write_ff3a,
	gb_apu_write_ff3b,
	gb_apu_write_ff3c,
	gb_apu_write_ff3d,
	gb_apu_write_ff3e,
	gb_apu_write_ff3f,
	
	// 0xff40
	gb_ppu_write_ff40,
	gb_ppu_write_ff41,
	gb_ppu_write_ff42,
	gb_ppu_write_ff43,
	NULL,
	gb_ppu_write_ff45,
	gb_dma_write_ff46,
	gb_ppu_write_ff47,
	gb_ppu_write_ff48,
	gb_ppu_write_ff49,
	gb_ppu_write_ff4a,
	gb_ppu_write_ff4b,
	NULL,
	NULL,
	NULL,
	NULL,
	
	// 0xff50
	gb_bios_write_ff50,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	
	// 0xff60
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	
	// 0xff70
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

uint8_t gb_mmu_read8_dma(uint16_t address) {
	switch((address >> 12) & 0x0f) {
		case 0x0:
			if(address < 0x100 && !GB_BIOS.biosDisabled) {
				return GB_BIOS.biosData[address];
			}

			return GB_CARTRIDGE.controller.rom_readFunc(address);

		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
			return GB_CARTRIDGE.controller.rom_readFunc(address);

		case 0x8:
		case 0x9:
			return gb_ppu_read_vram(address);

		case 0xA:
		case 0xB:
			return GB_CARTRIDGE.controller.ram_readFunc(address);

		case 0xC:
		case 0xD:
		case 0xE:
			return GB.ram[address & 0x1fff];

		case 0xF:
			switch((address >> 8) & 0x0f) {
				case 0x0:
				case 0x1:
				case 0x2:
				case 0x3:
				case 0x4:
				case 0x5:
				case 0x6:
				case 0x7:
				case 0x8:
				case 0x9:
				case 0xA:
				case 0xB:
				case 0xC:
				case 0xD:
					return GB.ram[address & 0x1fff];

				case 0xE:
					if(address < 0xfea0) {
						return gb_ppu_read_oam(address);
					}
					
					return 0x00;

				case 0xF:
					switch((address >> 7) & 0x01) {
						case 0: // 0xFF00-0xFF7F
							if(gb_mmu_ioreg_readFuncs[address & 0x7f]) {
								return gb_mmu_ioreg_readFuncs[address & 0x7f]();
							}

							return 0xff;

						case 1: // 0xFF80-0xFFFF
							return GB.hram[address & 0x7f];
					}
			}
	}

	// Dead code, but removes a warning
	return 0xff;
}

uint8_t gb_mmu_read8(uint16_t address) {
	if(GB_DMA.bufferFilled && address < 0xff00) {
		if(gbemu_options.printDmaAccessWarnings) {
			printf("Warning: memory read access outside HRAM during OAM DMA.\n");
		}

		if(!gbemu_options.enableDmaAccess) {
			return 0xff;
		}
	}

	return gb_mmu_read8_dma(address);
}

void gb_mmu_write8(uint16_t address, uint8_t value) {
	if(GB_DMA.bufferFilled && address < 0xff00) {
		if(gbemu_options.printDmaAccessWarnings) {
			printf("Warning: memory write access outside HRAM during OAM DMA.\n");
		}

		if(!gbemu_options.enableDmaAccess) {
			return;
		}
	}

	switch((address >> 12) & 0x0f) {
		case 0x0:
			if(address >= 0x100 || GB_BIOS.biosDisabled) {
				GB_CARTRIDGE.controller.rom_writeFunc(address, value);
			}

			break;

		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
			GB_CARTRIDGE.controller.rom_writeFunc(address, value);
			break;

		case 0x8:
		case 0x9:
			gb_ppu_write_vram(address, value);

			break;

		case 0xA:
		case 0xB:
			GB_CARTRIDGE.controller.ram_writeFunc(address, value);
			break;

		case 0xC:
		case 0xD:
		case 0xE:
			GB.ram[address & 0x1fff] = value;
			break;

		case 0xF:
			switch((address >> 8) & 0x0f) {
				case 0x0:
				case 0x1:
				case 0x2:
				case 0x3:
				case 0x4:
				case 0x5:
				case 0x6:
				case 0x7:
				case 0x8:
				case 0x9:
				case 0xA:
				case 0xB:
				case 0xC:
				case 0xD:
					GB.ram[address & 0x1fff] = value;
					break;

				case 0xE:
					if(address < 0xfea0) {
						gb_ppu_write_oam(address, value);
					}

					break;

				case 0xF:
					switch((address >> 7) & 0x01) {
						case 0: // 0xFF00-0xFF7F
							if(gb_mmu_ioreg_writeFuncs[address & 0x7f]) {
								gb_mmu_ioreg_writeFuncs[address & 0x7f](value);
							}

							break;

						case 1: // 0xFF80-0xFFFF
							GB.hram[address & 0x7f] = value;
							break;
					}
			}
	}
}
