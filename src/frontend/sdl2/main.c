#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "core/core.h"
#include "core/joypad.h"
#include "frontend/frontend.h"

#define C_BIOS_FILE_SIZE 256
#define C_MAX_ROM_FILE_SIZE 8388608
#define C_MAX_SRAM_FILE_SIZE 131072

static SDL_Window *s_window;
static SDL_Surface *s_bufferSurface;
static int s_windowScale;
static void *s_biosData;
static void *s_romData;
static size_t s_romSize;
static void *s_sramData;
static size_t s_sramSize;
static int s_frameCounter;
static uint64_t s_frameTimer;

struct ts_frontendConfiguration {
    char *a_biosFileName;
    char *a_romFileName;
    char *a_sramFileName;
    int a_screenScale;
};

static int init(int p_argc, char *p_argv[]);

static void setDefaultConfiguration(
    struct ts_frontendConfiguration *p_configuration
);

static int readCommandLineParameters(
    struct ts_frontendConfiguration *p_configuration,
    int p_argc,
    char *p_argv[]
);

static int loadFiles(
    struct ts_frontendConfiguration *p_configuration
);

static void *loadFile(const char *p_fileName, size_t *p_fileSize);

int main(int p_argc, char *p_argv[]) {
    if(init(p_argc, p_argv) != 0) {
        return 1;
    }

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "Error: SDL initialization failed.\n");
        return 1;
    }

    s_window = SDL_CreateWindow(
        "gbemu",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        160 * s_windowScale,
        144 * s_windowScale,
        0
    );

    if(s_window == NULL) {
        fprintf(stderr, "Error: SDL window creation failed.\n");
        return 1;
    }

    s_bufferSurface = SDL_CreateRGBSurface(
        0,
        160,
        144,
        32,
        0,
        0,
        0,
        0
    );

    if(s_bufferSurface == NULL) {
        fprintf(stderr, "Error: SDL buffer surface creation failed.\n");
        return 1;
    }

    if(coreInit() != 0) {
        return 1;
    }

    coreSetBios(s_biosData);

    if(coreSetRom(s_romData, s_romSize) != 0) {
        return 1;
    }

    coreReset();

    while(true) {
        coreStep();
    }

    return 0;
}

void frontendRenderFrame(const uint32_t *p_frameBuffer) {
    uint64_t l_currentTime = SDL_GetTicks64();

    if((l_currentTime - s_frameTimer) >= 1000) {
        char l_titleBuffer[100];

        sprintf(l_titleBuffer, "gbemu (%d fps)", s_frameCounter);

        SDL_SetWindowTitle(s_window, l_titleBuffer);

        s_frameCounter = 1;
        s_frameTimer = l_currentTime;
    } else {
        s_frameCounter++;
    }

    memcpy(s_bufferSurface->pixels, p_frameBuffer, 160 * 144 * 4);
    SDL_BlitScaled(s_bufferSurface, NULL, SDL_GetWindowSurface(s_window), NULL);
    SDL_UpdateWindowSurface(s_window);

    SDL_Event l_event;

    while(SDL_PollEvent(&l_event) == 1) {
        switch(l_event.type) {
            case SDL_WINDOWEVENT:
                if(l_event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    exit(0);
                }

                break;

            case SDL_KEYDOWN:
                if(l_event.key.keysym.sym == SDLK_RIGHT) {
                    coreJoypadSetInput(E_JOYPADINPUT_RIGHT, true);
                } else if(l_event.key.keysym.sym == SDLK_LEFT) {
                    coreJoypadSetInput(E_JOYPADINPUT_LEFT, true);
                } else if(l_event.key.keysym.sym == SDLK_UP) {
                    coreJoypadSetInput(E_JOYPADINPUT_UP, true);
                } else if(l_event.key.keysym.sym == SDLK_DOWN) {
                    coreJoypadSetInput(E_JOYPADINPUT_DOWN, true);
                } else if(l_event.key.keysym.sym == SDLK_z) {
                    coreJoypadSetInput(E_JOYPADINPUT_A, true);
                } else if(l_event.key.keysym.sym == SDLK_x) {
                    coreJoypadSetInput(E_JOYPADINPUT_B, true);
                } else if(l_event.key.keysym.sym == SDLK_LSHIFT) {
                    coreJoypadSetInput(E_JOYPADINPUT_SELECT, true);
                } else if(l_event.key.keysym.sym == SDLK_RETURN) {
                    coreJoypadSetInput(E_JOYPADINPUT_START, true);
                }

                break;

            case SDL_KEYUP:
                if(l_event.key.keysym.sym == SDLK_RIGHT) {
                    coreJoypadSetInput(E_JOYPADINPUT_RIGHT, false);
                } else if(l_event.key.keysym.sym == SDLK_LEFT) {
                    coreJoypadSetInput(E_JOYPADINPUT_LEFT, false);
                } else if(l_event.key.keysym.sym == SDLK_UP) {
                    coreJoypadSetInput(E_JOYPADINPUT_UP, false);
                } else if(l_event.key.keysym.sym == SDLK_DOWN) {
                    coreJoypadSetInput(E_JOYPADINPUT_DOWN, false);
                } else if(l_event.key.keysym.sym == SDLK_z) {
                    coreJoypadSetInput(E_JOYPADINPUT_A, false);
                } else if(l_event.key.keysym.sym == SDLK_x) {
                    coreJoypadSetInput(E_JOYPADINPUT_B, false);
                } else if(l_event.key.keysym.sym == SDLK_LSHIFT) {
                    coreJoypadSetInput(E_JOYPADINPUT_SELECT, false);
                } else if(l_event.key.keysym.sym == SDLK_RETURN) {
                    coreJoypadSetInput(E_JOYPADINPUT_START, false);
                }

                break;

            default:
                break;
        }
    }

    SDL_Delay(15);
}

static void setDefaultConfiguration(
    struct ts_frontendConfiguration *p_configuration
) {
    p_configuration->a_biosFileName = NULL;
    p_configuration->a_romFileName = NULL;
    p_configuration->a_sramFileName = NULL;
    p_configuration->a_screenScale = 1;
}

static int readCommandLineParameters(
    struct ts_frontendConfiguration *p_configuration,
    int p_argc,
    char *p_argv[]
) {
    bool l_flagBios = false;
    bool l_flagRom = false;
    bool l_flagSram = false;
    bool l_flagScale = false;

    for(
        int l_currentParameterIndex = 1;
        l_currentParameterIndex < p_argc;
        l_currentParameterIndex++
    ) {
        if(l_flagBios) {
            p_configuration->a_biosFileName = p_argv[l_currentParameterIndex];
            l_flagBios = false;
        } else if(l_flagRom) {
            p_configuration->a_romFileName = p_argv[l_currentParameterIndex];
            l_flagRom = false;
        } else if(l_flagSram) {
            p_configuration->a_sramFileName = p_argv[l_currentParameterIndex];
            l_flagSram = false;
        } else if(l_flagScale) {
            char *l_end;

            p_configuration->a_screenScale = strtol(
                p_argv[l_currentParameterIndex],
                &l_end,
                10
            );

            if(l_end == p_argv[l_currentParameterIndex]) {
                fprintf(
                    stderr,
                    "Error: failed to parse number \"%s\".\n",
                    p_argv[l_currentParameterIndex]
                );

                return 1;
            }

            l_flagScale = false;
        } else if(strcmp(p_argv[l_currentParameterIndex], "--bios") == 0) {
            l_flagBios = true;
        } else if(strcmp(p_argv[l_currentParameterIndex], "--rom") == 0) {
            l_flagRom = true;
        } else if(strcmp(p_argv[l_currentParameterIndex], "--sram") == 0) {
            l_flagSram = true;
        } else if(strcmp(p_argv[l_currentParameterIndex], "--scale") == 0) {
            l_flagScale = true;
        } else {
            fprintf(
                stderr,
                "Error: unknown command-line parameter \"%s\".\n",
                p_argv[l_currentParameterIndex]
            );

            return 1;
        }
    }

    if(l_flagBios) {
        fprintf(stderr, "Error: expected file name after \"--bios\".\n");
    } else if(l_flagRom) {
        fprintf(stderr, "Error: expected file name after \"--rom\".\n");
    } else if(l_flagSram) {
        fprintf(stderr, "Error: expected file name after \"--sram\".\n");
    } else if(l_flagSram) {
        fprintf(stderr, "Error: expected integer after \"--scale\".\n");
    } else if(p_configuration->a_biosFileName == NULL) {
        fprintf(stderr, "Error: no BIOS file name given.\n");
    } else if(p_configuration->a_romFileName == NULL) {
        fprintf(stderr, "Error: no ROM file name given.\n");
    } else {
        return 0;
    }

    return 1;
}

static int init(int p_argc, char *p_argv[]) {
    struct ts_frontendConfiguration l_configuration;

    setDefaultConfiguration(&l_configuration);

    if(readCommandLineParameters(&l_configuration, p_argc, p_argv)) {
        return 1;
    }

    if(loadFiles(&l_configuration) != 0) {
        return 1;
    }

    s_windowScale = l_configuration.a_screenScale;
    s_frameCounter = 0;
    s_frameTimer = SDL_GetTicks64();

    return 0;
}

static int loadFiles(
    struct ts_frontendConfiguration *p_configuration
) {
    size_t l_fileSize = C_BIOS_FILE_SIZE;

    s_biosData = loadFile(p_configuration->a_biosFileName, &l_fileSize);

    if(l_fileSize != C_BIOS_FILE_SIZE) {
        fprintf(stderr, "Error: invalid BIOS file size.\n");
        return 1;
    }

    if(s_biosData == NULL) {
        fprintf(stderr, "Error: failed to read BIOS file.\n");
        return 1;
    }

    l_fileSize = C_MAX_ROM_FILE_SIZE;

    s_romData = loadFile(p_configuration->a_romFileName, &l_fileSize);

    if(s_romData == NULL) {
        fprintf(stderr, "Error: failed to read ROM file.\n");
        return 1;
    }

    s_romSize = l_fileSize;

    // TODO: load SRAM

    return 0;
}

static void *loadFile(const char *p_fileName, size_t *p_fileSize) {
    FILE *l_file = fopen(p_fileName, "rb");

    if(l_file == NULL) {
        return NULL;
    }

    fseek(l_file, 0, SEEK_END);

    size_t l_fileSize = (size_t)ftell(l_file);

    fseek(l_file, 0, SEEK_SET);

    if(l_fileSize > *p_fileSize) {
        fclose(l_file);
        return NULL;
    }

    void *l_buffer = malloc(l_fileSize);

    size_t l_bytesRead = fread(l_buffer, 1, l_fileSize, l_file);

    fclose(l_file);

    if(l_bytesRead < l_fileSize) {
        free(l_buffer);
        return NULL;
    }

    *p_fileSize = l_fileSize;

    return l_buffer;
}
