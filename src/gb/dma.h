#ifndef __GB_DMA_H_INCLUDED__
#define __GB_DMA_H_INCLUDED__

#include <gb/gb.h>

#define GB_DMA (GB.dma)

typedef struct {
	uint8_t sourceRegister;
	int startCountdown;

	uint16_t source;
	unsigned int destination;
	uint8_t buffer;
	bool bufferFilled;
} gb_dma_t;

void gb_dma_init();
void gb_dma_cycle();

uint8_t gb_dma_read_ff46();
void gb_dma_write_ff46(uint8_t value);

#endif
