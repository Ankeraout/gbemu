#ifndef __GB_SERIAL_H_INCLUDED__
#define __GB_SERIAL_H_INCLUDED__

#include <stdbool.h>
#include <stdint.h>

#include <gb/gb.h>

#define GB_SERIAL (GB.serial)

typedef struct {
	uint8_t sb;
	bool transferInProgress;
	bool internalClock;
	uint8_t transferCounter;
} gb_serial_t;

void gb_serial_init();
void gb_serial_cycle();

uint8_t gb_serial_read_ff01();
void gb_serial_write_ff01(uint8_t value);
uint8_t gb_serial_read_ff02();
void gb_serial_write_ff02(uint8_t value);

#endif
