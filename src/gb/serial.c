#include <gb/gb.h>

void gb_serial_init() {
	GB_SERIAL.internalClock = false;
	GB_SERIAL.sb = 0;
	GB_SERIAL.transferCounter = 0;
	GB_SERIAL.transferInProgress = false;
}

uint8_t gb_serial_read_ff01() {
	return GB_SERIAL.sb;
}

void gb_serial_write_ff01(uint8_t value) {
	GB_SERIAL.sb = value;

	if(gbemu_options.redirectSerial) {
		if(gbemu_options.serialOutputHex) {
			fprintf(gbemu_options.serialOutputFile, "%02x", value);
		} else {
			fputc(value, gbemu_options.serialOutputFile);
		}

		fflush(stdout);
	}
}

uint8_t gb_serial_read_ff02() {
	uint8_t tmp = 0x7e;

	if(GB_SERIAL.transferInProgress) tmp |= 0x80;
	if(GB_SERIAL.internalClock) tmp |= 0x01;

	return tmp;
}

void gb_serial_write_ff02(uint8_t value) {
	GB_SERIAL.transferInProgress = ((value & 0x80) != 0);
	GB_SERIAL.internalClock = ((value & 0x01) != 0);
	
	if(!GB_SERIAL.transferCounter == 0 && ((value & 0x80) != 0)) {
		GB_SERIAL.transferCounter = 8;
	}
}

void gb_serial_cycle() {
	// TODO
}
