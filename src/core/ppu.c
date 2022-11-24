#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "core/ppu.h"
#include "frontend/frontend.h"

#define C_PPU_VRAM_SIZE 8192
#define C_PPU_OAM_SIZE 256

enum te_corePpuMode {
    E_CORE_PPU_MODE_HBLANK,
    E_CORE_PPU_MODE_VBLANK,
    E_CORE_PPU_MODE_OAMSCAN,
    E_CORE_PPU_MODE_DRAWING
};

static uint8_t s_corePpuVramData[C_PPU_VRAM_SIZE];
static uint8_t s_corePpuOamData[C_PPU_OAM_SIZE];

static const uint32_t s_screenPalette[4] = {
    0x00000000,
    0x55555555,
    0xaaaaaaaa,
    0xffffffff
};

static uint32_t s_frameBuffer[160 * 144];
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

static inline void corePpuUpdatePalette(
    uint8_t p_paletteRegisterValue,
    uint32_t *p_palette
);
static inline void corePpuDraw(void);

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
    /*if(!s_lcdEnable) {
        s_lx = 0;
        s_ly = 0;
        return;
    }*/

    s_lx += 4;

    if(s_ly < 144) {
        if(s_lx == 80) {
            s_mode = E_CORE_PPU_MODE_DRAWING;
        } else if(s_lx == 252) {
            s_mode = E_CORE_PPU_MODE_HBLANK;
        }
    }

    if(s_lx == 456) {
        s_ly++;
        s_lx = 0;

        if(s_ly == 144) {
            s_mode = E_CORE_PPU_MODE_VBLANK;
            corePpuDraw();
        } else {
            if(s_ly == 154) {
                s_ly = 0;
            }

            s_mode = E_CORE_PPU_MODE_OAMSCAN;
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
    return s_corePpuVramData[p_address & 0x1fff];
}

uint8_t corePpuReadOam(uint16_t p_address) {
    return s_corePpuOamData[p_address & 0x00ff];
}

void corePpuWriteIo(uint16_t p_address, uint8_t p_value) {
    if(p_address == 0xff40) {
        s_lcdc = p_value;
        s_lcdEnable = true;
        s_windowTileMapOffset = ((p_value & 0x40) == 0) ? 0x1800 : 0x1c00;
        s_windowEnable = (p_value & 0x20) != 0;
        s_bgTileSetOffset = ((p_value & 0x10) == 0) ? 0x0800 : 0x0000;
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
    s_corePpuVramData[p_address & 0x1fff] = p_value;
}

void corePpuWriteOam(uint16_t p_address, uint8_t p_value) {
    s_corePpuOamData[p_address & 0x00ff] = p_value;
}

static inline void corePpuUpdatePalette(
    uint8_t p_paletteRegisterValue,
    uint32_t *p_palette
) {
    int l_colorIndexes[4] = {
        p_paletteRegisterValue >> 6,
        (p_paletteRegisterValue >> 4) & 0x03,
        (p_paletteRegisterValue >> 2) & 0x03,
        p_paletteRegisterValue & 0x03,
    };

    for(int l_colorIndex = 0; l_colorIndex < 4; l_colorIndex++) {
        p_palette[l_colorIndex] = s_screenPalette[l_colorIndexes[l_colorIndex]];
    }
}

static inline void corePpuDraw(void) {
    // Draw background
    int l_backgroundY = s_scy;
    int l_pixelOffset = 0;

    for(int l_row = 0; l_row < 144; l_row++) {
        int l_backgroundX = s_scx;
        int l_bgMapY = l_backgroundY >> 3;
        int l_bgTileY = l_backgroundY & 0x07;

        for(int l_col = 0; l_col < 160; l_col++) {
            int l_bgMapX = l_backgroundX >> 3;
            int l_bgTileX = l_backgroundX & 0x07;
            int l_bgMapOffset = s_bgTileMapOffset | (l_bgMapY << 5) | l_bgMapX;
            int l_tileOffset = s_bgTileSetOffset | (l_bgTileY << 4) | (l_bgTileX << 1);

            uint8_t l_tileLow = s_corePpuVramData[l_tileOffset];
            uint8_t l_tileHigh = s_corePpuVramData[l_tileOffset | 1];
            uint8_t l_shift = 7 - l_bgTileX;
            uint8_t l_pixelLow = (l_tileLow >> l_shift) & 0x01;
            uint8_t l_pixelHigh = (l_tileHigh >> l_shift) & 0x01;
            uint8_t l_pixel = l_pixelLow | (l_pixelHigh << 1);

            s_frameBuffer[l_pixelOffset++] = s_backgroundPalette[l_pixel];

            l_backgroundX++;
        }

        l_backgroundY++;
    }

    printf("frame\n");
    fflush(stdout);

    frontendRenderFrame(s_frameBuffer);
}
