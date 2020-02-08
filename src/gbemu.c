#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL2/SDL.h>

#include <gb/gb.h>

#define BIOS_SIZE 256
#define MIN_ROM_SIZE 32768
#define MAX_ROM_SIZE 8388608

gbemu_options_t gbemu_options;

size_t getFileSize(FILE *file);
int readFile(FILE *file, uint8_t *buffer, size_t size);
void limitFps();

static inline void mainLoop_handleKeyboardEvent(bool keyDown, SDL_Keycode keyCode) {
	switch(keyCode) {
        case SDLK_UP:
            GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_UP] = keyDown;
            break;

        case SDLK_DOWN:
            GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_DOWN] = keyDown;
            break;
            
        case SDLK_LEFT:
            GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_LEFT] = keyDown;
            break;
            
        case SDLK_RIGHT:
            GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_RIGHT] = keyDown;
            break;
            
        case SDLK_x:
            GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_A] = keyDown;
            break;
            
        case SDLK_c:
            GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_B] = keyDown;
            break;
            
        case SDLK_RETURN:
            GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_START] = keyDown;
            break;
            
        case SDLK_LSHIFT:
            GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_SELECT] = keyDown;
            break;

        default: 
            break;
    }
}

int main(int argc, char *argv[]) {
    FILE *biosFile;
    FILE *romFile;
    size_t biosFileSize;
    size_t romFileSize;
    uint8_t biosData[256];
    uint8_t *romData;
	SDL_Window *window = NULL;
	bool mainLoop_exit = false; 
	
	struct {
		bool flag_rom;
		bool flag_save;
		bool flag_bios;
		bool flag_maxCycles;
		bool flag_redirectSerialFile;
		bool flag_screenScale;
		char *biosFileName;
		char *saveFileName;
		char *romFileName;
	} args;

	bool showUsage = false;

	args.flag_rom = false;
	args.flag_save = false;
	args.flag_bios = false;
	args.flag_maxCycles = false;
	args.flag_redirectSerialFile = false;
	args.flag_screenScale = false;
	args.biosFileName = NULL;
	args.saveFileName = NULL;
	args.romFileName = NULL;

	gbemu_options.disableFpsLimit = false;
	gbemu_options.printCartridgeInfo = false;
	gbemu_options.printOamAccessWarnings = false;
	gbemu_options.printVramAccessWarnings = false;
	gbemu_options.redirectSerial = false;
	gbemu_options.disableGraphics = false;
	gbemu_options.disableSramLoad = false;
	gbemu_options.disableSramSave = false;
	gbemu_options.serialOutputHex = false;
	gbemu_options.printDmaAccessWarnings = false;
	gbemu_options.printDmaDebug = false;
	gbemu_options.enableDmaAccess = false;
	gbemu_options.maxClocks = 0;
	gbemu_options.serialOutputFile = NULL;
	gbemu_options.screenScale = 1;

	for(int i = 1; i < argc; i++) {
		char *arg = argv[i];
			
		if(args.flag_rom) {
			args.flag_rom = false;
			args.romFileName = arg;
		} else if(args.flag_save) {
			args.flag_save = false;
			args.saveFileName = arg;
		} else if(args.flag_bios) {
			args.flag_bios = false;
			args.biosFileName = arg;
		} else if(args.flag_maxCycles) {
			args.flag_maxCycles = false;

			if(!sscanf(arg, "%u", &gbemu_options.maxClocks)) {
				gbemu_error("Failed to parse --max-clocks option value");
			}
		} else if(args.flag_redirectSerialFile) {
			args.flag_redirectSerialFile = false;
			gbemu_options.redirectSerial = true;

			gbemu_options.serialOutputFile = fopen(arg, "wb");

			if(!gbemu_options.serialOutputFile) {
				gbemu_error("Failed to open serial output file.");
			}
		} else if(args.flag_screenScale) {
			args.flag_screenScale = false;

			if(!sscanf(arg, "%u", &gbemu_options.screenScale)) {
				gbemu_error("Failed to parse --screen-scale option value.");
			}
		} else {
			if(strcmp(arg, "--rom") == 0) {
				args.flag_rom = true;
			} else if(strcmp(arg, "--save") == 0) {
				args.flag_save = true;
			} else if(strcmp(arg, "--bios") == 0) {
				args.flag_bios = true;
			} else if(strcmp(arg, "--disable-fps-limit") == 0) {
				gbemu_options.disableFpsLimit = true;
			} else if(strcmp(arg, "--enable-oam-warnings") == 0) {
				gbemu_options.printOamAccessWarnings = true;
			} else if(strcmp(arg, "--enable-vram-warnings") == 0) {
				gbemu_options.printVramAccessWarnings = true;
			} else if(strcmp(arg, "--enable-ppu-access-warnings") == 0) {
				gbemu_options.printOamAccessWarnings = true;
				gbemu_options.printVramAccessWarnings = true;
			} else if(strcmp(arg, "--enable-dma-access-warnings") == 0) {
				gbemu_options.printDmaAccessWarnings = true;
			} else if(strcmp(arg, "--enable-dma-debug") == 0) {
				gbemu_options.printDmaDebug = true;
			} else if(strcmp(arg, "--enable-dma-access") == 0) {
				gbemu_options.enableDmaAccess = true;
			} else if(strcmp(arg, "--print-cartridge-info") == 0) {
				gbemu_options.printCartridgeInfo = true;
			} else if(strcmp(arg, "--redirect-serial-stdout") == 0) {
				if(gbemu_options.redirectSerial) {
					gbemu_error("Serial output was redirected more than once.");
				}

				gbemu_options.redirectSerial = true;
				gbemu_options.serialOutputFile = stdout;
			} else if(strcmp(arg, "--redirect-serial-file") == 0) {
				if(gbemu_options.redirectSerial) {
					gbemu_error("Serial output was redirected more than once.");
				}

				args.flag_redirectSerialFile = true;
			} else if(strcmp(arg, "--disable-graphics") == 0) {
				gbemu_options.disableGraphics = true;
			} else if(strcmp(arg, "--disable-sram-load") == 0) {
				gbemu_options.disableSramLoad = true;
			} else if(strcmp(arg, "--disable-sram-save") == 0) {
				gbemu_options.disableSramSave = true;
			} else if(strcmp(arg, "--serial-output-hex") == 0) {
				gbemu_options.serialOutputHex = true;
			} else if(strcmp(arg, "--max-clocks") == 0) {
				args.flag_maxCycles = true;
			} else if(strcmp(arg, "--screen-scale") == 0) {
				args.flag_screenScale = true;
			} else {
				fprintf(stderr, "Unknown program argument \"%s\"\n", arg);
			}
		}
	}

	if(args.romFileName == NULL) {
		showUsage = true;
	} else if(args.biosFileName == NULL) {
		showUsage = true;
	}

    if(showUsage) {
        printf("Usage:\n");
        printf("%s <options>\n", argv[0]);
		printf("\n");
		printf("Available options:\n");
		printf("\t--rom <rom_file_path>: Mandatory, specifies the path to the ROM file.\n");
		printf("\t--save <save_file_path>: Not required, specifies the path to the save file.\n");
		printf("\t\tIf no save file is specified, then the save file path will be the path of the ROM file plus \".sav\" extension.\n");
		printf("\t--bios <bios_file_path>: Mandatory, specifies the path to the BIOS file.\n");
		printf("\t--disable-fps-limit: Disables the framerate limiter.\n");
		printf("\t--enable-oam-warnings: Shows a warning message everytime the OAM is accessed during STAT mode 2 or 3.\n");
		printf("\t--enable-vram-warnings: Shows a warning message everytime the VRAM is accessed during STAT mode 3.\n");
		printf("\t--enable-ppu-access-warnings: Equivalent to --enable-oam-warnings --enable-vram-warnings.\n");
		printf("\t--enable-dma-access-warnings: Shows a warning message for every RAM access outside HRAM during OAM DMA.\n");
		printf("\t--enable-dma-debug: Shows messages when OAM DMA is started or finishes.\n");
		printf("\t--enable-dma-access: Enables memory accesses outside HRAM during OAM DMA.\n");
		printf("\t--print-cartridge-info: Prints cartridge header information.\n");
		printf("\t--redirect-serial-stdout: Redirects serial output to stdout.\n");
		printf("\t--redirect-serial-file <file>: Redirects serial output to the given file.\n");
		printf("\t--disable-graphics: Disables the graphics to speed up emulation (useful for tests).\n");
		printf("\t--disable-sram-load: Prevents the emulator from loading the save file.\n");
		printf("\t--disable-sram-save: Prevents the emulator from writing the save file.\n");
		printf("\t--serial-output-hex: Writes serial output as hex characters.\n");
		printf("\t--max-clocks <clocks>: Defines the maximum amount of CPU clocks before the emulator exits.\n");
		printf("\t--screen-scale <scale>: The screen size multiplier. The default value is 1.\n");
		printf("\t\tIf this value is set to 0, then the emulator will never exit.\n");
        return EXIT_FAILURE;
    }

	if(args.saveFileName == NULL) {
		size_t romFileNameLength = strlen(args.romFileName);
		args.saveFileName = malloc(romFileNameLength + 5);
		strcpy(args.saveFileName, args.romFileName);
		strcpy(args.saveFileName + romFileNameLength, ".sav");
	}

    // Read BIOS file
    biosFile = fopen(args.biosFileName, "rb");

    if(!biosFile) {
        fprintf(stderr, "Failed to open BIOS file.\n");
        return EXIT_FAILURE;
    }

    biosFileSize = getFileSize(biosFile);

    if(biosFileSize != BIOS_SIZE) {
        fclose(biosFile);
        fprintf(stderr, "BIOS file has a wrong size.\n");
        return EXIT_FAILURE;
    }

    if(readFile(biosFile, biosData, BIOS_SIZE)) {
        fclose(biosFile);
        fprintf(stderr, "Failed to read BIOS file.\n");
        return EXIT_FAILURE;
    }

    fclose(biosFile);

    // Read ROM file
    romFile = fopen(args.romFileName, "rb");

    if(!romFile) {
        fprintf(stderr, "Failed to open ROM file.\n");
        return EXIT_FAILURE;
    }

    romFileSize = getFileSize(romFile);

    if(romFileSize > MAX_ROM_SIZE || romFileSize < MIN_ROM_SIZE) {
        fclose(romFile);
        fprintf(stderr, "Illegal ROM size.\n");
        return EXIT_FAILURE;
    }

    romData = malloc(romFileSize);

    if(!romData) {
        fclose(romFile);
        fprintf(stderr, "Failed to allocate memory for ROM\n");
        return EXIT_FAILURE;
    }

    if(readFile(romFile, romData, romFileSize)) {
        fclose(romFile);
        fprintf(stderr, "Failed to read ROM file.\n");
        return EXIT_FAILURE;
    }

    fclose(romFile);

	if(!gbemu_options.disableGraphics) {
		// Initialize SDL
		if(SDL_Init(SDL_INIT_VIDEO)) {
			fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
			return EXIT_FAILURE;
		}

		// Create window
		window = SDL_CreateWindow(
			"gbemu",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			GB_SCREEN_WIDTH * gbemu_options.screenScale,
			GB_SCREEN_HEIGHT * gbemu_options.screenScale,
			SDL_WINDOW_SHOWN
		);

		if(!window) {
			fprintf(stderr, "Failed to create emulator window.\n");
			return EXIT_FAILURE;
		}
	}

    if(gb_init(window, biosData, romData, romFileSize)) {
        fprintf(stderr, "Game Boy initialization failed.\n");
        return EXIT_FAILURE;
    }

	if(GB_CARTRIDGE.saveSize > 0) {
		if(gb_loadSaveFile(args.saveFileName)) {
			gbemu_error("An error occurred while loading save file.");
		}
	}

	// Main loop
	while(!mainLoop_exit) {
		if(!gbemu_options.disableGraphics) {
			SDL_Event event;

			while(SDL_PollEvent(&event)) {
				switch(event.type) {
					case SDL_WINDOWEVENT:
						switch(event.window.event) {
							case SDL_WINDOWEVENT_CLOSE:
								mainLoop_exit = true;
						}
						break;

					case SDL_KEYUP:
						mainLoop_handleKeyboardEvent(false, event.key.keysym.sym);
						break;

					case SDL_KEYDOWN:
						mainLoop_handleKeyboardEvent(true, event.key.keysym.sym);
						break;

					default:
						break;
				}
			}
		}

        // Do one game frame
		gb_frame();

		// Limit FPS
		if(!gbemu_options.disableFpsLimit) {
			limitFps();
		}

		// Update joypad
		gb_joypad_update();

		if(!gbemu_options.disableGraphics) {
			// Update game window
			SDL_UpdateWindowSurface(window);
		}
	}

	if(GB_CARTRIDGE.saveSize > 0) {
		if(gb_saveSaveFile(args.saveFileName)) {
			gbemu_error("An error occurred while saving the save file.");
		}
	}

	if(gbemu_options.redirectSerial) {
		fclose(gbemu_options.serialOutputFile);
	}

	// Quit SDL
	SDL_Quit();

    return EXIT_SUCCESS;
}

size_t getFileSize(FILE *file) {
    size_t oldPosition = ftell(file);
    
    fseek(file, 0L, SEEK_END);

    size_t fileSize = ftell(file);

    fseek(file, oldPosition, SEEK_SET);

    return fileSize;
}

int readFile(FILE *file, uint8_t *buffer, size_t size) {
	if(fread(buffer, 1, size, file) != size) {
		return -1;
	} else {
		return 0;
	}
}

void limitFps() {
	static Uint32 lastFrameTime = 0;
	const unsigned int framerateLimit = 60;
	const unsigned int delayBetweenFrames = 1000 / framerateLimit;

	while((SDL_GetTicks() - lastFrameTime) < delayBetweenFrames) {
		SDL_Delay(delayBetweenFrames - (SDL_GetTicks() - lastFrameTime));
	}

	lastFrameTime = SDL_GetTicks();
}
