#ifndef __GB_CPU_H_INCLUDED__
#define __GB_CPU_H_INCLUDED__

#include <stdbool.h>
#include <stdint.h>

#include <gbemu.h>

#define GB_CPU (GB.cpu)

#define GB_INT_VBLANK 0x01
#define GB_INT_STAT 0x02
#define GB_INT_TIMER 0x04
#define GB_INT_SERIAL 0x08
#define GB_INT_JOYPAD 0x10

typedef struct {
	uint8_t a;
	bool flagZ;
	bool flagN;
	bool flagH;
	bool flagC;

	union {
		uint16_t bc;

		struct {
#ifdef GBEMU_BIG_ENDIAN
			uint8_t b;
			uint8_t c;
#else
			uint8_t c;
			uint8_t b;
#endif
		};
	};

	union {
		uint16_t de;

		struct {
#ifdef GBEMU_BIG_ENDIAN
			uint8_t d;
			uint8_t e;
#else
			uint8_t e;
			uint8_t d;
#endif
		};
	};

	union {
		uint16_t hl;

		struct {
#ifdef GBEMU_BIG_ENDIAN
			uint8_t h;
			uint8_t l;
#else
			uint8_t l;
			uint8_t h;
#endif
		};
	};
	
	uint16_t pc;
	uint16_t sp;
	bool halted;
	bool stopped;
	bool ime;
	bool ime_enableNextCycle;
	uint8_t ioreg_if;
} gb_cpu_t;

void gb_cpu_init();
void gb_cpu_instruction();

uint8_t gb_cpu_read_ff0f();
void gb_cpu_write_ff0f(uint8_t value);

#endif
