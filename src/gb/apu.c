#include <string.h>

#include <SDL2/SDL.h>

#include <gb/gb.h>

#define PI 3.14159265358979323846
#define PI2 (2 * PI)

static const uint8_t gb_apu_wavePatternRamInitialValues[16] = {
	0x84, 0x40, 0x43, 0xaa,
	0x2d, 0x78, 0x92, 0x3c,
	0x60, 0x59, 0x59, 0xb0,
	0x34, 0xb8, 0x2e, 0xda
};

float audioBuffer[AUDIO_BUFFER_SIZE];
int audioBufferWriteCursor = 0;
int audioBufferReadCursor = 0;
volatile int audioBufferSize = 0;

void gb_apu_init() {
	memset(&GB_APU, 0, sizeof(GB_APU));
	
	for(unsigned int i = 0; i < 16; i++) {
		GB_APU.wavePattern[i] = gb_apu_wavePatternRamInitialValues[i];
	}
}

static float generateSquareWave(double x) {
	if(fmod(x, 1) >= 0.5) {
		return 1;
	} else {
		return -1;
	}
}

static inline float gb_apu_generateSampleChannel1(double timing) {
	return generateSquareWave(timing * GB_APU.c1_frequency) * GB_APU.c1_volume;
}

static inline float gb_apu_generateSampleChannel2(double timing) {
	return generateSquareWave(timing * GB_APU.c2_frequency) * GB_APU.c2_volume;
}

static inline float gb_apu_generateSampleChannel3(double timing) {
	UNUSED_PARAMETER(timing);
	return 0;
}

static inline float gb_apu_generateSampleChannel4(double timing) {
	UNUSED_PARAMETER(timing);
	return 0;
}

static inline void gb_apu_generateSample() {
	while(audioBufferSize == AUDIO_BUFFER_SIZE) {
		SDL_Delay(1);
	}

	if(audioBufferSize < AUDIO_BUFFER_SIZE) {
		double timing = (double)GB.clocks / (double)(1048576 * 4);
		float sample = 0;

		if(GB_APU.c1_onFlag) {
			sample += gb_apu_generateSampleChannel1(timing) * .25f;
		}

		if(GB_APU.c2_onFlag) {
			sample += gb_apu_generateSampleChannel2(timing) * .25f;
		}

		if(GB_APU.c3_onFlag) {
			sample += gb_apu_generateSampleChannel3(timing) * .25f;
		}

		if(GB_APU.c4_onFlag) {
			sample += gb_apu_generateSampleChannel4(timing) * .25f;
		}

		audioBuffer[audioBufferWriteCursor] = sample;

		audioBufferWriteCursor++;
		audioBufferWriteCursor %= AUDIO_BUFFER_SIZE;
		audioBufferSize++;
	}
}

void gb_apu_lengthCounterClock() {
	if(GB_APU.c1_onFlag && GB_APU.c1_length) {
		GB_APU.c1_length--;

		if(!GB_APU.c1_length) {
			if(GB_APU.c1_stopWhenLengthZero) {
				GB_APU.c1_onFlag = false;
			}

			GB_APU.c1_length = 64;
		}
	}

	if(GB_APU.c2_onFlag && GB_APU.c2_length) {
		GB_APU.c2_length--;

		if(!GB_APU.c2_length) {
			if(GB_APU.c2_stopWhenLengthZero) {
				GB_APU.c2_onFlag = false;
			}

			GB_APU.c2_length = 64;
		}
	}

	if(GB_APU.c3_onFlag && GB_APU.c3_length) {
		GB_APU.c3_length--;

		if(!GB_APU.c3_length) {
			if(GB_APU.c3_stopWhenLengthZero) {
				GB_APU.c3_onFlag = false;
			}
			
			GB_APU.c3_length = 256;
		}
	}

	if(GB_APU.c4_onFlag && GB_APU.c4_length) {
		GB_APU.c4_length--;

		if(!GB_APU.c4_length) {
			if(GB_APU.c4_stopWhenLengthZero) {
				GB_APU.c4_onFlag = false;
			}
			
			GB_APU.c4_length = 64;
		}
	}
}

void gb_apu_volumeEnvelopeClock() {
	if(GB_APU.c1_volumeSweepLength) {
		GB_APU.c1_volumeSweepCounter++;

		if(GB_APU.c1_volumeSweepLength == GB_APU.c1_volumeSweepCounter) {
			GB_APU.c1_volume += GB_APU.c1_volumeSweepDirection / 15.0;

			if(GB_APU.c1_volume > 1) {
				GB_APU.c1_volume = 1;
			} else if(GB_APU.c1_volume < 0) {
				GB_APU.c1_volume = 0;
			}

			GB_APU.c1_volumeSweepCounter = 0;
		}
	}

	if(GB_APU.c2_volumeSweepLength) {
		GB_APU.c2_volumeSweepCounter++;

		if(GB_APU.c2_volumeSweepLength == GB_APU.c2_volumeSweepCounter) {
			GB_APU.c2_volume += GB_APU.c2_volumeSweepDirection / 15.0;

			if(GB_APU.c2_volume > 1) {
				GB_APU.c2_volume = 1;
			} else if(GB_APU.c2_volume < 0) {
				GB_APU.c2_volume = 0;
			}

			GB_APU.c2_volumeSweepCounter = 0;
		}
	}
}

void gb_apu_sweepClock() {

}

void gb_apu_frameSequencerClock() {
	GB_APU.frameSequencerClockCount++;

	if((GB_APU.frameSequencerClockCount & 0x01) == 0) {
		gb_apu_lengthCounterClock();
	}

	if((GB_APU.frameSequencerClockCount & 0x07) == 7) {
		gb_apu_volumeEnvelopeClock();
	}

	if((GB_APU.frameSequencerClockCount & 0x03) == 2) {
		gb_apu_sweepClock();
	}
}

void gb_apu_cycle() {
	static uint64_t lastClock = 0;
	uint64_t clock = (((uint64_t)GB.clocks & 0x003fffff) * (uint64_t)AUDIO_SAMPLE_RATE) >> 22;

	if(GB_APU.enabled) {
		GB_APU.clock_512hz++;

		if((GB_APU.clock_512hz & 0x07ff) == 0) {
			gb_apu_frameSequencerClock();
		}
	}

	if(clock != lastClock) {
		gb_apu_generateSample();
		lastClock = clock;
	}
}

float gb_apu_getAudioSample() {
	if(audioBufferSize == 0) {
		return 0;
	} else {
		float sample = audioBuffer[audioBufferReadCursor];
		
		audioBufferReadCursor++;
		audioBufferReadCursor %= AUDIO_BUFFER_SIZE;
		audioBufferSize--;

		return sample;
	}
}

uint8_t gb_apu_read_ff10() {
	return GB_APU.nr10 | 0x80;
}

uint8_t gb_apu_read_ff11() {
	return GB_APU.nr11 | 0x3f;
}

uint8_t gb_apu_read_ff12() {
	return GB_APU.nr12;
}

uint8_t gb_apu_read_ff14() {
	return GB_APU.nr14 | 0xbf;
}

uint8_t gb_apu_read_ff16() {
	return GB_APU.nr21 | 0x3f;
}

uint8_t gb_apu_read_ff17() {
	return GB_APU.nr22;
}

uint8_t gb_apu_read_ff19() {
	return GB_APU.nr24 | 0xbf;
}

uint8_t gb_apu_read_ff1a() {
	return GB_APU.nr30 | 0x7f;
}

uint8_t gb_apu_read_ff1c() {
	return GB_APU.nr32 | 0x9f;
}

uint8_t gb_apu_read_ff1e() {
	return GB_APU.nr34 | 0xbf;
}

uint8_t gb_apu_read_ff20() {
	return GB_APU.nr41 | 0xff;
}

uint8_t gb_apu_read_ff21() {
	return GB_APU.nr42;
}

uint8_t gb_apu_read_ff22() {
	return GB_APU.nr43;
}

uint8_t gb_apu_read_ff23() {
	return GB_APU.nr44 | 0xbf;
}

uint8_t gb_apu_read_ff24() {
	return GB_APU.nr50;
}

uint8_t gb_apu_read_ff25() {
	return GB_APU.nr51;
}

uint8_t gb_apu_read_ff26() {
	uint8_t value = GB_APU.nr52 & 0x80;

	if(GB_APU.c1_onFlag) value |= 0x01;
	if(GB_APU.c2_onFlag) value |= 0x02;
	if(GB_APU.c3_onFlag) value |= 0x04;
	if(GB_APU.c4_onFlag) value |= 0x08;

	return value | 0x70;
}

uint8_t gb_apu_read_ff30() {
	return GB_APU.wavePattern[0x00];
}

uint8_t gb_apu_read_ff31() {
	return GB_APU.wavePattern[0x01];
}

uint8_t gb_apu_read_ff32() {
	return GB_APU.wavePattern[0x02];
}

uint8_t gb_apu_read_ff33() {
	return GB_APU.wavePattern[0x03];
}

uint8_t gb_apu_read_ff34() {
	return GB_APU.wavePattern[0x04];
}

uint8_t gb_apu_read_ff35() {
	return GB_APU.wavePattern[0x05];
}

uint8_t gb_apu_read_ff36() {
	return GB_APU.wavePattern[0x06];
}

uint8_t gb_apu_read_ff37() {
	return GB_APU.wavePattern[0x07];
}

uint8_t gb_apu_read_ff38() {
	return GB_APU.wavePattern[0x08];
}

uint8_t gb_apu_read_ff39() {
	return GB_APU.wavePattern[0x09];
}

uint8_t gb_apu_read_ff3a() {
	return GB_APU.wavePattern[0x0a];
}

uint8_t gb_apu_read_ff3b() {
	return GB_APU.wavePattern[0x0b];
}

uint8_t gb_apu_read_ff3c() {
	return GB_APU.wavePattern[0x0c];
}

uint8_t gb_apu_read_ff3d() {
	return GB_APU.wavePattern[0x0d];
}

uint8_t gb_apu_read_ff3e() {
	return GB_APU.wavePattern[0x0e];
}

uint8_t gb_apu_read_ff3f() {
	return GB_APU.wavePattern[0x0f];
}

void gb_apu_write_ff10(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr10 = value | 0x80;
		GB_APU.c1_sweepTime = (value >> 4) & 0x07;
		GB_APU.c1_sweepMultiplier = (value >> 3) & 0x01 ? -1 : 1;
		GB_APU.c1_sweepShiftCount = value & 0x07;
	}
}

void gb_apu_write_ff11(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr11 = value | 0x3f;
		GB_APU.c1_length = 64 - (value & 0x3f);
	}
}

void gb_apu_write_ff12(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr12 = value;
	}
}

void gb_apu_write_ff13(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr13 = value;
		GB_APU.c1_frequency = 131072 / (2048 - (GB_APU.nr13 | ((GB_APU.nr14 << 8) & 0x0700)));
	}
}

void gb_apu_write_ff14(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr14 = value;

		if(value & 0x80) {
			GB_APU.c1_onFlag = true;
			GB_APU.c1_length = 64 - (GB_APU.nr11 & 0x3f);
			GB_APU.c1_stopWhenLengthZero = (value & 0x40);
			GB_APU.c1_volume = (GB_APU.nr12 >> 4) / 15.0;
			GB_APU.c1_volumeSweepLength = GB_APU.nr12 & 0x07;
			GB_APU.c1_volumeSweepCounter = 0;
			GB_APU.c1_volumeSweepDirection = GB_APU.nr12 & 0x08 ? 1 : -1;
			GB_APU.c1_frequency = 131072 / (2048 - (GB_APU.nr13 | ((GB_APU.nr14 << 8) & 0x0700)));
		}
	}
}

void gb_apu_write_ff16(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr21 = value;
		GB_APU.c2_length = 64 - (value & 0x3f);
	}
}

void gb_apu_write_ff17(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr22 = value;
	}
}

void gb_apu_write_ff18(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr23 = value;
		GB_APU.c2_frequency = 131072 / (2048 - (GB_APU.nr23 | ((GB_APU.nr24 << 8) & 0x0700)));
	}
}

void gb_apu_write_ff19(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr24 = value;

		if(value & 0x80) {
			GB_APU.c2_onFlag = true;
			GB_APU.c2_length = 64 - (GB_APU.nr21 & 0x3f);
			GB_APU.c2_stopWhenLengthZero = (value & 0x40);
			GB_APU.c2_volume = (GB_APU.nr22 >> 4) / 15.0;
			GB_APU.c2_volumeSweepLength = GB_APU.nr22 & 0x07;
			GB_APU.c2_volumeSweepCounter = 0;
			GB_APU.c2_volumeSweepDirection = GB_APU.nr22 & 0x08 ? 1 : -1;
			GB_APU.c2_frequency = 131072 / (2048 - (GB_APU.nr23 | ((GB_APU.nr24 << 8) & 0x0700)));
		}
	}
}

void gb_apu_write_ff1a(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr30 = value;
	}
}

void gb_apu_write_ff1b(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr31 = value;
		GB_APU.c3_length = 256 - value;
	}
}

void gb_apu_write_ff1c(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr32 = value;
	}
}

void gb_apu_write_ff1d(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr33 = value;
	}
}

void gb_apu_write_ff1e(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr34 = value;

		if(value & 0x80) {
			GB_APU.c3_onFlag = true;
			GB_APU.c3_length = 256 - GB_APU.nr31;
			GB_APU.c3_stopWhenLengthZero = (value & 0x40);
		}
	}
}

void gb_apu_write_ff20(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr41 = value;
		GB_APU.c4_length = 64 - (value & 0x3f);
		GB_APU.c4_stopWhenLengthZero = (value & 0x40);
	}
}

void gb_apu_write_ff21(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr42 = value;
	}
}

void gb_apu_write_ff22(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr43 = value;
	}
}

void gb_apu_write_ff23(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr44 = value;

		if(value & 0x80) {
			GB_APU.c4_onFlag = true;
			GB_APU.c4_length = 64 - (GB_APU.nr41 & 0x3f);
		}
	}
}

void gb_apu_write_ff24(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr50 = value;
	}
}

void gb_apu_write_ff25(uint8_t value) {
	if(GB_APU.enabled) {
		GB_APU.nr51 = value;
	}
}

void gb_apu_write_ff26(uint8_t value) {
	GB_APU.nr52 = value;
	GB_APU.enabled = (value & 0x80) != 0;

	if(!GB_APU.enabled) {
		GB_APU.nr10 = 0;
		GB_APU.nr11 = 0;
		GB_APU.nr12 = 0;
		GB_APU.nr13 = 0;
		GB_APU.nr14 = 0;
		GB_APU.nr21 = 0;
		GB_APU.nr22 = 0;
		GB_APU.nr23 = 0;
		GB_APU.nr24 = 0;
		GB_APU.nr30 = 0;
		GB_APU.nr31 = 0;
		GB_APU.nr32 = 0;
		GB_APU.nr33 = 0;
		GB_APU.nr34 = 0;
		GB_APU.nr41 = 0;
		GB_APU.nr42 = 0;
		GB_APU.nr43 = 0;
		GB_APU.nr44 = 0;
		GB_APU.nr50 = 0;
		GB_APU.nr51 = 0;

		GB_APU.c1_onFlag = false;
		GB_APU.c2_onFlag = false;
		GB_APU.c3_onFlag = false;
		GB_APU.c4_onFlag = false;

		GB_APU.c1_length = 0;
		GB_APU.c2_length = 0;
		GB_APU.c3_length = 0;
		GB_APU.c4_length = 0;

		GB_APU.c1_stopWhenLengthZero = false;
		GB_APU.c2_stopWhenLengthZero = false;
		GB_APU.c3_stopWhenLengthZero = false;
		GB_APU.c4_stopWhenLengthZero = false;

		GB_APU.clock_512hz = 0;
		GB_APU.frameSequencerClockCount = 0;
	}
}

void gb_apu_write_ff30(uint8_t value) {
	GB_APU.wavePattern[0x00] = value;
}

void gb_apu_write_ff31(uint8_t value) {
	GB_APU.wavePattern[0x01] = value;
}

void gb_apu_write_ff32(uint8_t value) {
	GB_APU.wavePattern[0x02] = value;
}

void gb_apu_write_ff33(uint8_t value) {
	GB_APU.wavePattern[0x03] = value;
}

void gb_apu_write_ff34(uint8_t value) {
	GB_APU.wavePattern[0x04] = value;
}

void gb_apu_write_ff35(uint8_t value) {
	GB_APU.wavePattern[0x05] = value;
}

void gb_apu_write_ff36(uint8_t value) {
	GB_APU.wavePattern[0x06] = value;
}

void gb_apu_write_ff37(uint8_t value) {
	GB_APU.wavePattern[0x07] = value;
}

void gb_apu_write_ff38(uint8_t value) {
	GB_APU.wavePattern[0x08] = value;
}

void gb_apu_write_ff39(uint8_t value) {
	GB_APU.wavePattern[0x09] = value;
}

void gb_apu_write_ff3a(uint8_t value) {
	GB_APU.wavePattern[0x0a] = value;
}

void gb_apu_write_ff3b(uint8_t value) {
	GB_APU.wavePattern[0x0b] = value;
}

void gb_apu_write_ff3c(uint8_t value) {
	GB_APU.wavePattern[0x0c] = value;
}

void gb_apu_write_ff3d(uint8_t value) {
	GB_APU.wavePattern[0x0d] = value;
}

void gb_apu_write_ff3e(uint8_t value) {
	GB_APU.wavePattern[0x0e] = value;
}

void gb_apu_write_ff3f(uint8_t value) {
	GB_APU.wavePattern[0x0f] = value;
}
