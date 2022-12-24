#ifndef __INCLUDE_CORE_PPU_H__
#define __INCLUDE_CORE_PPU_H__

#include <stdint.h>

void corePpuReset(void);
void corePpuCycle(void);
void corePpuCycleDouble(void);
uint8_t corePpuReadIo(uint16_t p_address);
uint8_t corePpuReadVram(uint16_t p_address);
uint8_t corePpuReadOam(uint16_t p_address);
void corePpuWriteIo(uint16_t p_address, uint8_t p_value);
void corePpuWriteVram(uint16_t p_address, uint8_t p_value);
void corePpuWriteOam(uint16_t p_address, uint8_t p_value);
void corePpuWriteOamDma(unsigned int p_index, uint8_t p_value);

#endif
