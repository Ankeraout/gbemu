#ifndef __GB_JOYPAD_H_INCLUDED__
#define __GB_JOYPAD_H_INCLUDED__

#include <stdbool.h>

#include <gb/gb.h>

#define GB_JOYPAD (GB.joypad)

enum {
	GB_JOYPAD_KEY_DOWN,
	GB_JOYPAD_KEY_UP,
	GB_JOYPAD_KEY_LEFT,
	GB_JOYPAD_KEY_RIGHT,

	GB_JOYPAD_KEY_START,
	GB_JOYPAD_KEY_SELECT,
	GB_JOYPAD_KEY_B,
	GB_JOYPAD_KEY_A
};

typedef struct {
	bool keyPressed[8];
	bool selectButtonKeys;
	bool selectDirectionKeys;
	uint8_t oldValue;
} gb_joypad_t;

void gb_joypad_init();
void gb_joypad_update(void);
uint8_t gb_joypad_read_ff00();
void gb_joypad_write_ff00(uint8_t value);

#endif
