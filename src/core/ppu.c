#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "core/cpu.h"
#include "core/ppu.h"
#include "frontend/frontend.h"

#define C_PPU_VRAM_SIZE 8192
#define C_PPU_OAM_SIZE 256
#define C_PPU_SCREEN_WIDTH 160
#define C_PPU_SCREEN_HEIGHT 144
#define C_PPU_OBJECT_COUNT 40
#define C_PPU_MAX_OBJECTS_PER_SCANLINE 10
#define C_PPU_OAMFLAG_OBP1 (1 << 4)
#define C_PPU_OAMFLAG_FLIP_X (1 << 5)
#define C_PPU_OAMFLAG_FLIP_Y (1 << 6)
#define C_PPU_OAMFLAG_UNDER_BG (1 << 7)
#define C_PPU_OAMFLAG_MASK (C_PPU_OAMFLAG_OBP1 | C_PPU_OAMFLAG_UNDER_BG)

enum te_corePpuMode {
    E_CORE_PPU_MODE_HBLANK,
    E_CORE_PPU_MODE_VBLANK,
    E_CORE_PPU_MODE_OAMSCAN,
    E_CORE_PPU_MODE_DRAWING
};

static uint8_t s_corePpuVramData[C_PPU_VRAM_SIZE];
static uint8_t s_corePpuOamData[C_PPU_OAM_SIZE];

static const uint32_t s_screenPalette[4] = {
    0xffffffff,
    0xaaaaaaaa,
    0x55555555,
    0x00000000
};

static uint8_t s_lcdc;
static bool s_lcdEnable;
static int s_windowTileMapOffset;
static bool s_windowEnable;
static int s_bgTileSetOffset;
static int s_bgTileMapOffset;
static int s_objHeight;
static bool s_objEnable;
static bool s_bgWindowEnable;
static uint8_t s_stat;
static bool s_interruptEnableLyc;
static bool s_interruptEnableOam;
static bool s_interruptEnableVblank;
static bool s_interruptEnableHblank;
static uint8_t s_scy;
static uint8_t s_scx;
static uint8_t s_lyc;
static uint8_t s_wy;
static uint8_t s_wx;
static uint8_t s_bgp;
static uint32_t s_backgroundPalette[4];
static uint8_t s_obp[2];
static uint32_t s_objectPalette[2][4];
static int s_ly;
static int s_lx;
static int s_mode;
static int s_tileIdInverter;
static uint32_t s_frameBuffer[C_PPU_SCREEN_WIDTH * C_PPU_SCREEN_HEIGHT];
static bool s_windowTriggered;
static int s_windowCounter;
static bool s_line153quirk;

static inline void corePpuUpdatePalette(
    uint8_t p_paletteRegisterValue,
    uint32_t *p_palette
);
static inline void corePpuDrawLine(void);
static inline bool corePpuCanAccessVram(void);
static inline bool corePpuCanAccessOam(void);
static inline void corePpuCheckLyc(void);

void corePpuReset(void) {
    for(int l_vramOffset = 0; l_vramOffset < C_PPU_VRAM_SIZE; l_vramOffset++) {
        s_corePpuVramData[l_vramOffset] = 0;
    }

    for(int l_oamOffset = 0; l_oamOffset < C_PPU_OAM_SIZE; l_oamOffset++) {
        s_corePpuOamData[l_oamOffset] = 0;
    }

    corePpuWriteIo(0xff40, 0x00);
    corePpuWriteIo(0xff41, 0x00);
    corePpuWriteIo(0xff47, 0x00);
    corePpuWriteIo(0xff48, 0x00);
    corePpuWriteIo(0xff49, 0x00);

    s_scy = 0;
    s_scx = 0;
    s_ly = 0;
    s_lx = 0;
    s_lyc = 0;
    s_wy = 0;
    s_wx = 0;
}

void corePpuCycle(void) {
    if(!s_lcdEnable) {
        s_lx = 0;
        s_ly = C_PPU_SCREEN_HEIGHT;
        s_mode = E_CORE_PPU_MODE_VBLANK;
        s_line153quirk = true; // I think I read somewhere that the first scanline is not rendered when re-enabling LCD?
        s_windowTriggered = false;
        return;
    }

    s_lx += 4;

    if(s_line153quirk) {
        if(s_lx == 456) {
            s_lx = 0;
            s_mode = E_CORE_PPU_MODE_OAMSCAN;
            s_line153quirk = false;

            if(s_interruptEnableOam) {
                coreCpuRequestInterrupt(E_CPUINTERRUPT_STAT);
            }
        }
    } else {
        if(s_ly < C_PPU_SCREEN_HEIGHT) {
            if(s_lx == 80) {
                s_mode = E_CORE_PPU_MODE_DRAWING;
            } else if(s_lx == 252) {
                s_mode = E_CORE_PPU_MODE_HBLANK;

                if(s_interruptEnableHblank) {
                    coreCpuRequestInterrupt(E_CPUINTERRUPT_STAT);
                }

                corePpuDrawLine();
            } else if(s_lx == 456) {
                if(s_ly < (C_PPU_SCREEN_HEIGHT - 1)) {
                    s_mode = E_CORE_PPU_MODE_OAMSCAN;
                    
                    if(s_interruptEnableOam) {
                        coreCpuRequestInterrupt(E_CPUINTERRUPT_STAT);
                    }
                } else {
                    s_mode = E_CORE_PPU_MODE_VBLANK;
                    
                    if(s_interruptEnableVblank) {
                        coreCpuRequestInterrupt(E_CPUINTERRUPT_STAT);
                    }

                    coreCpuRequestInterrupt(E_CPUINTERRUPT_VBLANK);

                    frontendRenderFrame(s_frameBuffer);
                }

                s_ly++;
                s_lx = 0;

                corePpuCheckLyc();
            }
        } else if(s_ly < 153) {
            if(s_lx == 456) {
                s_ly++;
                s_lx = 0;

                corePpuCheckLyc();
            }
        } else {
            s_line153quirk = true;
            s_windowTriggered = false;
            s_ly = 0;

            corePpuCheckLyc();
        }
    }
}

uint8_t corePpuReadIo(uint16_t p_address) {
    if(p_address == 0xff40) {
        return s_lcdc;
    } else if(p_address == 0xff41) {
        uint8_t l_returnValue = s_stat | s_mode;

        if(s_ly == s_lyc) {
            l_returnValue |= 0x04;
        }

        return l_returnValue;
    } else if(p_address == 0xff42) {
        return s_scy;
    } else if(p_address == 0xff43) {
        return s_scx;
    } else if(p_address == 0xff44) {
        return s_ly;
    } else if(p_address == 0xff45) {
        return s_lyc;
    } else if(p_address == 0xff47) {
        return s_bgp;
    } else if(p_address == 0xff48) {
        return s_obp[0];
    } else if(p_address == 0xff49) {
        return s_obp[1];
    } else if(p_address == 0xff4a) {
        return s_wy;
    } else if(p_address == 0xff4b) {
        return s_wx;
    } else {
        return 0xff;
    }
}

uint8_t corePpuReadVram(uint16_t p_address) {
    if(corePpuCanAccessVram()) {
        return s_corePpuVramData[p_address & 0x1fff];
    } else {
        return 0xff;
    }
}

uint8_t corePpuReadOam(uint16_t p_address) {
    if(corePpuCanAccessOam()) {
        // VBlank or HBlank
        return s_corePpuOamData[p_address & 0x00ff];
    } else {
        return 0xff;
    }
}

void corePpuWriteIo(uint16_t p_address, uint8_t p_value) {
    if(p_address == 0xff40) {
        s_lcdc = p_value;
        s_lcdEnable = (p_value & 0x80) != 0;
        s_windowTileMapOffset = ((p_value & 0x40) == 0) ? 0x1800 : 0x1c00;
        s_windowEnable = (p_value & 0x20) != 0;
        s_bgTileSetOffset = ((p_value & 0x10) == 0) ? 0x0800 : 0x0000;
        s_tileIdInverter = ((p_value & 0x10) == 0) ? 0x80 : 0x00;
        s_bgTileMapOffset = ((p_value & 0x08) == 0) ? 0x1800 : 0x1c00;
        s_objHeight = ((p_value & 0x04) == 0) ? 8 : 16;
        s_objEnable = (p_value & 0x02) != 0;
        s_bgWindowEnable = (p_value & 0x01) != 0;
    } else if(p_address == 0xff41) {
        s_stat = (p_value & 0x78) | 0x80;
        s_interruptEnableLyc = (p_value & 0x40) != 0;
        s_interruptEnableOam = (p_value & 0x20) != 0;
        s_interruptEnableVblank = (p_value & 0x10) != 0;
        s_interruptEnableHblank = (p_value & 0x08) != 0;
    } else if(p_address == 0xff42) {
        s_scy = p_value;
    } else if(p_address == 0xff43) {
        s_scx = p_value;
    } else if(p_address == 0xff45) {
        s_lyc = p_value;
        corePpuCheckLyc();
    } else if(p_address == 0xff47) {
        s_bgp = p_value;
        corePpuUpdatePalette(s_bgp, s_backgroundPalette);
    } else if(p_address == 0xff48) {
        s_obp[0] = p_value;
        corePpuUpdatePalette(s_obp[0], s_objectPalette[0]);
    } else if(p_address == 0xff49) {
        s_obp[1] = p_value;
        corePpuUpdatePalette(s_obp[1], s_objectPalette[1]);
    } else if(p_address == 0xff4a) {
        s_wy = p_value;
    } else if(p_address == 0xff4b) {
        s_wx = p_value;
    }
}

void corePpuWriteVram(uint16_t p_address, uint8_t p_value) {
    if(corePpuCanAccessVram()) {
        s_corePpuVramData[p_address & 0x1fff] = p_value;
    }
}

void corePpuWriteOam(uint16_t p_address, uint8_t p_value) {
    if(corePpuCanAccessOam()) {
        // VBlank or HBlank
        s_corePpuOamData[p_address & 0x00ff] = p_value;
    }
}

static inline void corePpuUpdatePalette(
    uint8_t p_paletteRegisterValue,
    uint32_t *p_palette
) {
    int l_colorIndexes[4] = {
        p_paletteRegisterValue & 0x03,
        (p_paletteRegisterValue >> 2) & 0x03,
        (p_paletteRegisterValue >> 4) & 0x03,
        p_paletteRegisterValue >> 6,
    };

    for(int l_colorIndex = 0; l_colorIndex < 4; l_colorIndex++) {
        p_palette[l_colorIndex] = s_screenPalette[l_colorIndexes[l_colorIndex]];
    }
}

void corePpuWriteOamDma(unsigned int p_index, uint8_t p_value) {
    s_corePpuOamData[p_index] = p_value;
}

static inline void corePpuDrawLinePrepareBackground(int p_backgroundBuffer[]) {
    uint8_t l_backgroundY = s_scy + s_ly;
    int l_mapY = l_backgroundY >> 3;
    int l_tileY = l_backgroundY & 0x07;
    uint8_t l_backgroundX = s_scx;

    for(int l_col = 0; l_col < 160; l_col++) {
        int l_mapX = l_backgroundX >> 3;
        int l_tileX = l_backgroundX & 0x07;
        int l_mapOffset = s_bgTileMapOffset | (l_mapY << 5) | l_mapX;
        int l_tileId = s_corePpuVramData[l_mapOffset] ^ s_tileIdInverter;
        int l_tileOffset = s_bgTileSetOffset + ((l_tileId << 4) | (l_tileY << 1));
        int l_tileLow = s_corePpuVramData[l_tileOffset];
        int l_tileHigh = s_corePpuVramData[l_tileOffset | 1];
        int l_shift = 7 - l_tileX;
        uint8_t l_pixelLow = (l_tileLow >> l_shift) & 0x01;
        uint8_t l_pixelHigh = (l_tileHigh >> l_shift) & 0x01;
        uint8_t l_pixel = l_pixelLow | (l_pixelHigh << 1);

        p_backgroundBuffer[l_col] = l_pixel;

        l_backgroundX++;
    }
}

static inline void corePpuDrawLinePrepareWindow(int p_backgroundBuffer[]) {
    if(s_windowTriggered) {
        if(s_wx <= 166) {
            s_windowCounter++;
        }
    } else if(s_ly == s_wy) {
        s_windowTriggered = true;
        s_windowCounter = 0;
    } else {
        return;
    }

    int l_mapY = s_windowCounter >> 3;
    int l_tileY = s_windowCounter & 0x07;
    int l_windowX = 7 - s_wx;

    for(int l_col = 0; l_col < 160; l_col++) {
        if(l_windowX < 0) {
            l_windowX++;
            continue;
        }

        int l_mapX = l_windowX >> 3;
        int l_tileX = l_windowX & 0x07;
        int l_mapIndex = (l_mapY << 5) | l_mapX;
        int l_tileId = s_corePpuVramData[s_windowTileMapOffset | l_mapIndex] ^ s_tileIdInverter;
        int l_tileOffset = s_bgTileSetOffset + ((l_tileId << 4) | (l_tileY << 1));
        int l_tileLow = s_corePpuVramData[l_tileOffset];
        int l_tileHigh = s_corePpuVramData[l_tileOffset | 1];
        int l_shift = 7 - l_tileX;
        uint8_t l_pixelLow = (l_tileLow >> l_shift) & 0x01;
        uint8_t l_pixelHigh = (l_tileHigh >> l_shift) & 0x01;
        uint8_t l_pixel = l_pixelLow | (l_pixelHigh << 1);

        p_backgroundBuffer[l_col] = l_pixel;

        l_windowX++;
    }
}

static inline void corePpuDrawLineDrawBackground(int p_backgroundBuffer[]) {
    int l_frameBufferIndex = s_ly * C_PPU_SCREEN_WIDTH;

    for(int l_col = 0; l_col < C_PPU_SCREEN_WIDTH; l_col++) {
        s_frameBuffer[l_frameBufferIndex++] = s_backgroundPalette[p_backgroundBuffer[l_col]];
    }
}

static inline void corePpuDrawLineDrawBackgroundObject(int p_backgroundBuffer[]) {
    int l_objectTable[C_PPU_MAX_OBJECTS_PER_SCANLINE];
    int l_objectCount = 0;
    int l_objectBuffer[C_PPU_SCREEN_WIDTH];

    memset(l_objectBuffer, 0, sizeof(l_objectBuffer));

    // Scan OAM
    for(int l_objectIndex = 0; l_objectIndex < C_PPU_OBJECT_COUNT; l_objectIndex++) {
        int l_objectOffset = l_objectIndex << 2;
        int l_objectYMin = s_corePpuOamData[l_objectOffset] - 16;
        int l_objectYMax = l_objectYMin + s_objHeight - 1;

        if((l_objectYMin <= s_ly) && (s_ly <= l_objectYMax)) {
            l_objectTable[l_objectCount++] = l_objectOffset;

            if(l_objectCount == C_PPU_MAX_OBJECTS_PER_SCANLINE) {
                break;
            }
        }
    }

    // Sort objects by increasing X coordinate (selection sort)
    for(int l_i = 0; l_i < (l_objectCount - 1); l_i++) {
        int l_objectOffset1 = l_objectTable[l_i];
        int l_objectX1 = s_corePpuOamData[l_objectOffset1 | 1];

        for(int l_j = l_i + 1; l_j < l_objectCount; l_j++) {
            int l_objectOffset2 = l_objectTable[l_j];
            int l_objectX2 = s_corePpuOamData[l_objectOffset2 | 1];

            if(l_objectX1 > l_objectX2) {
                int l_exchange = l_objectTable[l_i];
                l_objectTable[l_i] = l_objectTable[l_j];
                l_objectTable[l_j] = l_exchange;

                l_objectOffset1 = l_objectOffset2;
                l_objectX1 = l_objectX2;
            }
        }
    }

    // Draw each object in the object buffer
    for(int l_objectTableIndex = 0; l_objectTableIndex < l_objectCount; l_objectTableIndex++) {
        int l_objectOffset = l_objectTable[l_objectTableIndex];
        const int l_objectY = s_corePpuOamData[l_objectOffset++] - 16;
        int l_objectX = s_corePpuOamData[l_objectOffset++] - 8;
        int l_objectTileIndex = s_corePpuOamData[l_objectOffset++];

        // Enforce tile index
        if(s_objHeight == 16) {
            l_objectTileIndex &= 0xfe;
        }

        const int l_objectFlags = s_corePpuOamData[l_objectOffset];
        const int l_objectRow = s_ly - l_objectY;

        // Compute the row in the sprite
        int l_spriteRow;

        if((l_objectFlags & C_PPU_OAMFLAG_FLIP_Y) != 0) {
            l_spriteRow = s_objHeight - 1 - l_objectRow;
        } else {
            l_spriteRow = l_objectRow;
        }

        // Fetch tile
        int l_tileOffset = (l_objectTileIndex << 4) | (l_spriteRow << 1);
        int l_tileLow = s_corePpuVramData[l_tileOffset];
        int l_tileHigh = s_corePpuVramData[l_tileOffset | 1];
        
        // Draw tile pixels
        int l_shiftStart;
        int l_shiftDirection;
        int l_shiftEnd;

        if((l_objectFlags & C_PPU_OAMFLAG_FLIP_X) != 0) {
            l_shiftStart = 0;
            l_shiftDirection = 1;
            l_shiftEnd = 8;
        } else {
            l_shiftStart = 7;
            l_shiftDirection = -1;
            l_shiftEnd = -1;
        }

        for(int l_shift = l_shiftStart; l_shift != l_shiftEnd; l_shift += l_shiftDirection) {
            if(l_objectX < 0) {
                l_objectX++;
                continue;
            } else if(l_objectX >= C_PPU_SCREEN_WIDTH) {
                break;
            }

            int l_pixelLow = (l_tileLow >> l_shift) & 0x01;
            int l_pixelHigh = (l_tileHigh >> l_shift) & 0x01;
            int l_pixel = l_pixelLow | (l_pixelHigh << 1);
            int l_pixelFlags = l_objectFlags & C_PPU_OAMFLAG_MASK;

            if((l_pixel != 0) && (l_objectBuffer[l_objectX] == 0)) {
                l_objectBuffer[l_objectX] = l_pixel | l_pixelFlags;
            }

            l_objectX++;
        }
    }

    // Merge background and object layers
    int l_frameBufferIndex = C_PPU_SCREEN_WIDTH * s_ly;

    for(int l_x = 0; l_x < C_PPU_SCREEN_WIDTH; l_x++) {
        int l_backgroundPixelColor = p_backgroundBuffer[l_x];
        int l_objectPixel = l_objectBuffer[l_x];
        int l_objectPixelColor = l_objectPixel & 0x03;
        int l_objectPixelPalette = (l_objectPixel >> 4) & 0x01;
        bool l_objectPixelUnderBackground = (l_objectPixel & C_PPU_OAMFLAG_UNDER_BG) != 0;

        if(l_objectPixelColor == 0) {
            s_frameBuffer[l_frameBufferIndex++] = s_backgroundPalette[l_backgroundPixelColor];
        } else if(l_backgroundPixelColor == 0) {
            s_frameBuffer[l_frameBufferIndex++] = s_objectPalette[l_objectPixelPalette][l_objectPixelColor];
        } else if(l_objectPixelUnderBackground) {
            s_frameBuffer[l_frameBufferIndex++] = s_backgroundPalette[l_backgroundPixelColor];
        } else {
            s_frameBuffer[l_frameBufferIndex++] = s_objectPalette[l_objectPixelPalette][l_objectPixelColor];
        }
    }
}

static inline void corePpuDrawLine(void) {
    int l_backgroundBuffer[160];

    memset(l_backgroundBuffer, 0, sizeof(l_backgroundBuffer));

    if(s_bgWindowEnable) {
        corePpuDrawLinePrepareBackground(l_backgroundBuffer);

        if(s_windowEnable) {
            corePpuDrawLinePrepareWindow(l_backgroundBuffer);
        }
    }

    if(s_objEnable) {
        corePpuDrawLineDrawBackgroundObject(l_backgroundBuffer);
    } else {
        corePpuDrawLineDrawBackground(l_backgroundBuffer);
    }
}

static inline bool corePpuCanAccessVram(void) {
    return s_mode != E_CORE_PPU_MODE_DRAWING;
}

static inline bool corePpuCanAccessOam(void) {
    return (s_mode & 0x02) == 0;
}

static inline void corePpuCheckLyc(void) {
    static bool l_oldLycCondition = false;
    bool l_lycCondition = (s_ly == s_lyc);
    bool l_risingEdge = ((!l_oldLycCondition) && l_lycCondition);

    if(l_risingEdge && s_interruptEnableLyc) {
        coreCpuRequestInterrupt(E_CPUINTERRUPT_STAT);
    }

    l_oldLycCondition = l_lycCondition;
}
