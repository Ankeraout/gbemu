#include <gb/gb.h>

void gb_joypad_init() {
	for(int i = 0; i < 8; i++) {
		GB_JOYPAD.keyPressed[i] = false;
	}

	GB_JOYPAD.selectButtonKeys = false;
	GB_JOYPAD.selectDirectionKeys = false;
	GB_JOYPAD.oldValue = gb_joypad_read_ff00();
}

uint8_t gb_joypad_read_ff00() {
	uint8_t tmp = 0xff;

	if(GB_JOYPAD.selectDirectionKeys) {
		tmp &= ~0x20;

		if(GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_START]) tmp &= ~0x08;
		if(GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_SELECT]) tmp &= ~0x04;
		if(GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_B]) tmp &= ~0x02;
		if(GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_A]) tmp &= ~0x01;
	}

	if(GB_JOYPAD.selectButtonKeys) {
		tmp &= ~0x10;

		if(GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_DOWN]) tmp &= ~0x08;
		if(GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_UP]) tmp &= ~0x04;
		if(GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_LEFT]) tmp &= ~0x02;
		if(GB_JOYPAD.keyPressed[GB_JOYPAD_KEY_RIGHT]) tmp &= ~0x01;
	}

	return tmp;
}

void gb_joypad_write_ff00(uint8_t value) {
	GB_JOYPAD.selectDirectionKeys = ((value & 0x20) == 0);
	GB_JOYPAD.selectButtonKeys = ((value & 0x10) == 0);
}

void gb_joypad_update(void) {
	uint8_t currentValue = gb_joypad_read_ff00();
	uint8_t diff = GB_JOYPAD.oldValue & ~currentValue;

	if(diff & 0x0f) {
		GB_CPU.ioreg_if |= GB_INT_JOYPAD;
	}
}
