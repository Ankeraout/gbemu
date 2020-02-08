#include <gb/gb.h>

void gb_dma_init() {
	GB_DMA.bufferFilled = false;
	GB_DMA.startCountdown = 0;
	GB_DMA.source = 0x00ff;
}

void gb_dma_cycle() {
	if(GB_DMA.startCountdown > 0) {
		GB_DMA.startCountdown--;

		if(!GB_DMA.startCountdown) {
			GB_DMA.destination = 0x00;

			if(GB_DMA.sourceRegister >= 0xe0) {
				GB_DMA.source = (GB_DMA.sourceRegister << 8) & ~0x2000;
			} else {
				GB_DMA.source = GB_DMA.sourceRegister << 8;
			}

			GB_DMA.bufferFilled = false;
		}
	}

	if(GB_DMA.bufferFilled) {
		GB_PPU.oam[GB_DMA.destination++] = GB_DMA.buffer;
		GB_DMA.bufferFilled = false;
	}

	if((GB_DMA.source & 0xff) < 0xa0) {
		GB_DMA.buffer = gb_mmu_read8_dma(GB_DMA.source);
		GB_DMA.source++;
		GB_DMA.bufferFilled = true;

		if(gbemu_options.printDmaDebug && ((GB_DMA.source & 0xff) == 0xa0)) {
			printf("Finished OAM DMA.\n");
		}
	}
}

uint8_t gb_dma_read_ff46() {
	return GB_DMA.sourceRegister;

}

void gb_dma_write_ff46(uint8_t value) {
	if(gbemu_options.printDmaDebug) {
		printf("Started OAM DMA with source address 0x%02x00.\n", value);
	}

	GB_DMA.sourceRegister = value;
	GB_DMA.startCountdown = 2;
}
