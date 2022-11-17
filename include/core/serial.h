#ifndef __INCLUDE_CORE_SERIAL_H__
#define __INCLUDE_CORE_SERIAL_H__

#include <stdint.h>

void coreSerialReset(void);
uint8_t coreSerialRead(uint16_t p_address);
void coreSerialWrite(uint16_t p_address, uint8_t p_value);

#endif
