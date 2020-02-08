#ifndef __GB_TIMER_H_INCLUDED__
#define __GB_TIMER_H_INCLUDED__

#include <gb/gb.h>

#define GB_TIMER (GB.timer)

typedef struct {
	uint16_t div;
	uint8_t tima;
	uint8_t tma;
	uint8_t tac;
	bool enabled;
	int clockValue;
	bool timaReload;
	bool timaReload2;
} gb_timer_t;

void gb_timer_init();
void gb_timer_cycle();

uint8_t gb_timer_read_ff04();
uint8_t gb_timer_read_ff05();
uint8_t gb_timer_read_ff06();
uint8_t gb_timer_read_ff07();
void gb_timer_write_ff04(uint8_t value);
void gb_timer_write_ff05(uint8_t value);
void gb_timer_write_ff06(uint8_t value);
void gb_timer_write_ff07(uint8_t value);

#endif