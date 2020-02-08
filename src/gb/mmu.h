#ifndef __GB_MMU_H_INCLUDED__
#define __GB_MMU_H_INCLUDED__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <gb/gb.h>

uint8_t gb_mmu_read8(uint16_t address);
uint8_t gb_mmu_read8_dma(uint16_t address);
void gb_mmu_write8(uint16_t address, uint8_t value);
void gb_mmu_write8_dma(uint16_t address, uint8_t value);

static inline uint8_t gb_mmu_read8_cycle(uint16_t address) {
	uint8_t returnValue = gb_mmu_read8(address);
	gb_cycleNoCPU();
	return returnValue;
}

static inline void gb_mmu_write8_cycle(uint16_t address, uint8_t value) {
	gb_mmu_write8(address, value);
	gb_cycleNoCPU();
}

#endif
