#ifndef __GB_BIOS_H_INCLUDED__
#define __GB_BIOS_H_INCLUDED__

#include <stdbool.h>
#include <stdint.h>

#include <gb/gb.h>

#define GB_BIOS (GB.bios)

typedef struct {
	bool biosDisabled;
	uint8_t *biosData;
} gb_bios_t;

void gb_bios_init(uint8_t *biosData);

uint8_t gb_bios_read_ff50();
void gb_bios_write_ff50(uint8_t value);

#endif
