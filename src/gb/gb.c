#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <gb/gb.h>

gb_t GB;

int gb_init(SDL_Window *window, uint8_t *biosData, uint8_t *romData, size_t romSize) {
	memset(&GB, 0, sizeof(GB));

	gb_apu_init();
	gb_bios_init(biosData);

	if(gb_cartridge_init(romData, romSize)) {
		fprintf(stderr, "Cartridge initialization failed.\n");
		return EXIT_FAILURE;
	}

	gb_cpu_init();
	gb_dma_init();
	gb_joypad_init();
	
	if(gb_ppu_init(window)) {
		fprintf(stderr, "PPU initialization failed.\n");
		return EXIT_FAILURE;
	}

	gb_serial_init();
	gb_timer_init();

	return EXIT_SUCCESS;
}

void gb_cycleNoCPU() {
	gb_apu_cycle();
	gb_dma_cycle();
	gb_ppu_cycle();
	gb_timer_cycle();
	gb_serial_cycle();

	gb.clocks += 4;

	if(gbemu_options.maxClocks && gb.clocks > gbemu_options.maxClocks) {
		exit(0);
	}
}

void gb_frame() {
	while(true) {
		gb_cpu_instruction();

		// One frame has occurred!
		if(GB_PPU.frameFlag) {
			GB_PPU.frameFlag = false;
			return;
		}
	}
}

void gb_randomSaveFile() {
	srand(time(NULL));

	for(size_t i = 0; i < GB_CARTRIDGE.saveSize; i++) {
		GB_CARTRIDGE.saveData[i] = (uint8_t)rand();
	}
}

int gb_loadSaveFile(const char *saveFileName) {
	if(gbemu_options.disableSramLoad) {
		gb_randomSaveFile();

		return 0;
	}
	
	FILE *saveFile = fopen(saveFileName, "rb");
	
	if(saveFile == NULL) {
		// File probably does not exist. Ignore and load random data in SRAM.
		gb_randomSaveFile();

		return 0;
	} else if(getFileSize(saveFile) != GB_CARTRIDGE.saveSize) {
		fclose(saveFile);
		fprintf(stderr, "The size of the save file does not match the expected size. The save file is probably corrupt.");
		return 1;
	} else if(readFile(saveFile, GB_CARTRIDGE.saveData, GB_CARTRIDGE.saveSize)) {
		fclose(saveFile);
		fprintf(stderr, "Could not read save file.");
		return 1;
	}

	fclose(saveFile);
	return 0;
}

int gb_saveSaveFile(const char *saveFileName) {
	if(gbemu_options.disableSramSave) {
		return 0;
	}

	FILE *saveFile = fopen(saveFileName, "wb");

	if(saveFile == NULL) {
		fprintf(stderr, "Could not open save file for writing.\n");
		return 1;
	}

	if(fwrite(GB_CARTRIDGE.saveData, 1, GB_CARTRIDGE.saveSize, saveFile) != GB_CARTRIDGE.saveSize) {
		fclose(saveFile);
		fprintf(stderr, "An error occurred while writing save file.\n");
		return 1;
	}

	fclose(saveFile);
	return 0;
}
