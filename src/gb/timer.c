#include <gb/gb.h>

static const unsigned int gb_timer_inputClockSelectValues[4] = {
	(1 << 10),
	(1 << 4),
	(1 << 6),
	(1 << 8)
};

static const unsigned int gb_timer_inputClockBit[4] = {
	(1 << 9),
	(1 << 3),
	(1 << 5),
	(1 << 7)
};

static inline void gb_timer_increaseTima() {
	GB_TIMER.tima++;

	if(GB_TIMER.tima == 0) {
		GB_TIMER.timaReload = true;
	}
}

static inline void gb_timer_updateDiv(uint16_t newValue) {
	bool oldBit = GB_TIMER.div & gb_timer_inputClockBit[GB_TIMER.clockValue];
	bool newBit = newValue & gb_timer_inputClockBit[GB_TIMER.clockValue];

	GB_TIMER.div = newValue;

	GB_TIMER.timaReload2 = false;

	if(GB_TIMER.timaReload) {
		GB_TIMER.timaReload = false;
		GB_TIMER.tima = GB_TIMER.tma;

		// Request timer interrupt
		GB_CPU.ioreg_if |= GB_INT_TIMER;
		GB_TIMER.timaReload2 = true;
	}

	if(GB_TIMER.enabled) {
		if(oldBit && !newBit) {
			gb_timer_increaseTima();
		}
	}
}

void gb_timer_init() {
	GB_TIMER.clockValue = 0;
	GB_TIMER.enabled = false;
	GB_TIMER.div = 0;
	GB_TIMER.tima = 0;
	GB_TIMER.tma = 0;
	GB_TIMER.tac = 0xf8;
	GB_TIMER.timaReload = false;
	GB_TIMER.timaReload2 = false;
}

void gb_timer_cycle() {
	gb_timer_updateDiv(GB_TIMER.div + 4);
}

uint8_t gb_timer_read_ff04() {
	return (uint8_t)(GB_TIMER.div >> 8);
}

uint8_t gb_timer_read_ff05() {
	if(GB_TIMER.timaReload) {
		return 0x00;
	}

	return GB_TIMER.tima;
}

uint8_t gb_timer_read_ff06() {
	return GB_TIMER.tma;
}

uint8_t gb_timer_read_ff07() {
	return GB_TIMER.tac;
}

void gb_timer_write_ff04(uint8_t value) {
	UNUSED_PARAMETER(value);

	gb_timer_updateDiv(0);
}

void gb_timer_write_ff05(uint8_t value) {
	if(!GB_TIMER.timaReload2) {
		GB_TIMER.tima = value;

		if(GB_TIMER.timaReload) {
			GB_TIMER.timaReload = false;
		}
	}
}

void gb_timer_write_ff06(uint8_t value) {
	GB_TIMER.tma = value;

	if(GB_TIMER.timaReload2) {
		GB_TIMER.tima = value;
	}
}

void gb_timer_write_ff07(uint8_t value) {
	int oldClock = gb_timer_inputClockSelectValues[GB_TIMER.clockValue];
	bool oldEnabled = GB_TIMER.enabled;

	GB_TIMER.enabled = (value & 0x04) != 0;
	GB_TIMER.clockValue = value & 0x03;
	GB_TIMER.tac = value | 0xf8;

	if(oldEnabled) {
		if(!GB_TIMER.enabled) {
			if(GB_TIMER.div & gb_timer_inputClockBit[GB_TIMER.clockValue]) {
				gb_timer_increaseTima();
			}
		}
	} else {
		bool glitch = false;

		if(GB_TIMER.enabled) {
			glitch = ((GB_TIMER.div & (oldClock / 2)) != 0) && ((GB_TIMER.div & (gb_timer_inputClockSelectValues[GB_TIMER.clockValue] / 2)) == 0);
		} else {
			glitch = (GB_TIMER.div & (oldClock / 2)) != 0;
		}

		if(glitch) {
			gb_timer_increaseTima();
		}
	}
}
