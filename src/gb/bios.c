#include <stdbool.h>
#include <stdint.h>

#include <gb/gb.h>

void gb_bios_init(uint8_t *biosData) {
	GB_BIOS.biosDisabled = false;
	GB_BIOS.biosData = biosData;
}

uint8_t gb_bios_read_ff50() {
	if(GB_BIOS.biosDisabled) {
		return 0xff;
	} else {
		return 0xfe;
	}
}

void gb_bios_write_ff50(uint8_t value) {
	if(value & 0x01) {
		GB_BIOS.biosDisabled = true;
	}
}
