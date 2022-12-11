#include <stdint.h>
#include <stdio.h>

#include "core/serial.h"

void coreSerialReset(void) {

}

uint8_t coreSerialRead(uint16_t p_address) {

}

void coreSerialWrite(uint16_t p_address, uint8_t p_value) {
    printf("Serial write: 0x%02x\n", p_value);
}
