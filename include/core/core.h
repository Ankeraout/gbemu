#ifndef __INCLUDE_CORE_CORE_H__
#define __INCLUDE_CORE_CORE_H__

#include <stdint.h>

int coreInit(void);
void coreSetBios(const void *p_bios);
int coreSetRom(const void *p_rom, size_t p_size);
void coreReset(void);
void coreFrameAdvance(void);
void coreCycle(void);

#endif
