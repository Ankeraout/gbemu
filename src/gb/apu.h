#ifndef __GB_APU_H_INCLUDED__
#define __GB_APU_H_INCLUDED__

#include <stdbool.h>
#include <stdint.h>

#include <gb/gb.h>

#define GB_APU (GB.apu)

typedef struct {
	bool enabled;

	bool c1_onFlag;
	unsigned int c1_length;
	unsigned int c1_sweepTime;
	int c1_sweepMultiplier;
	unsigned int c1_sweepShiftCount;
	bool c1_stopWhenLengthZero;
	unsigned int c1_frequency;
	unsigned int c1_envelope;
	uint8_t nr10;
	uint8_t nr11;
	uint8_t nr12;
	uint8_t nr13;
	uint8_t nr14;

	bool c2_onFlag;
	unsigned int c2_length;
	bool c2_stopWhenLengthZero;
	uint8_t nr21;
	uint8_t nr22;
	uint8_t nr23;
	uint8_t nr24;

	bool c3_onFlag;
	unsigned int c3_length;
	bool c3_stopWhenLengthZero;
	uint8_t nr30;
	uint8_t nr31;
	uint8_t nr32;
	uint8_t nr33;
	uint8_t nr34;

	bool c4_onFlag;
	unsigned int c4_length;
	bool c4_stopWhenLengthZero;
	uint8_t nr41;
	uint8_t nr42;
	uint8_t nr43;
	uint8_t nr44;

	uint8_t nr50;
	uint8_t nr51;
	uint8_t nr52;

	uint8_t wavePattern[16];

	unsigned int clock_512hz;
	unsigned int frameSequencerClockCount;
} gb_apu_t;

void gb_apu_init();
void gb_apu_cycle();
float gb_apu_getAudioSample();

uint8_t gb_apu_read_ff10();
uint8_t gb_apu_read_ff11();
uint8_t gb_apu_read_ff12();
uint8_t gb_apu_read_ff14();
uint8_t gb_apu_read_ff16();
uint8_t gb_apu_read_ff17();
uint8_t gb_apu_read_ff19();
uint8_t gb_apu_read_ff1a();
uint8_t gb_apu_read_ff1c();
uint8_t gb_apu_read_ff1e();
uint8_t gb_apu_read_ff20();
uint8_t gb_apu_read_ff21();
uint8_t gb_apu_read_ff22();
uint8_t gb_apu_read_ff23();
uint8_t gb_apu_read_ff24();
uint8_t gb_apu_read_ff25();
uint8_t gb_apu_read_ff26();
uint8_t gb_apu_read_ff30();
uint8_t gb_apu_read_ff31();
uint8_t gb_apu_read_ff32();
uint8_t gb_apu_read_ff33();
uint8_t gb_apu_read_ff34();
uint8_t gb_apu_read_ff35();
uint8_t gb_apu_read_ff36();
uint8_t gb_apu_read_ff37();
uint8_t gb_apu_read_ff38();
uint8_t gb_apu_read_ff39();
uint8_t gb_apu_read_ff3a();
uint8_t gb_apu_read_ff3b();
uint8_t gb_apu_read_ff3c();
uint8_t gb_apu_read_ff3d();
uint8_t gb_apu_read_ff3e();
uint8_t gb_apu_read_ff3f();

void gb_apu_write_ff10(uint8_t value);
void gb_apu_write_ff11(uint8_t value);
void gb_apu_write_ff12(uint8_t value);
void gb_apu_write_ff13(uint8_t value);
void gb_apu_write_ff14(uint8_t value);
void gb_apu_write_ff16(uint8_t value);
void gb_apu_write_ff17(uint8_t value);
void gb_apu_write_ff18(uint8_t value);
void gb_apu_write_ff19(uint8_t value);
void gb_apu_write_ff1a(uint8_t value);
void gb_apu_write_ff1b(uint8_t value);
void gb_apu_write_ff1c(uint8_t value);
void gb_apu_write_ff1d(uint8_t value);
void gb_apu_write_ff1e(uint8_t value);
void gb_apu_write_ff20(uint8_t value);
void gb_apu_write_ff21(uint8_t value);
void gb_apu_write_ff22(uint8_t value);
void gb_apu_write_ff23(uint8_t value);
void gb_apu_write_ff24(uint8_t value);
void gb_apu_write_ff25(uint8_t value);
void gb_apu_write_ff26(uint8_t value);
void gb_apu_write_ff30(uint8_t value);
void gb_apu_write_ff31(uint8_t value);
void gb_apu_write_ff32(uint8_t value);
void gb_apu_write_ff33(uint8_t value);
void gb_apu_write_ff34(uint8_t value);
void gb_apu_write_ff35(uint8_t value);
void gb_apu_write_ff36(uint8_t value);
void gb_apu_write_ff37(uint8_t value);
void gb_apu_write_ff38(uint8_t value);
void gb_apu_write_ff39(uint8_t value);
void gb_apu_write_ff3a(uint8_t value);
void gb_apu_write_ff3b(uint8_t value);
void gb_apu_write_ff3c(uint8_t value);
void gb_apu_write_ff3d(uint8_t value);
void gb_apu_write_ff3e(uint8_t value);
void gb_apu_write_ff3f(uint8_t value);

#endif
