#ifndef __INCLUDE_CORE_HRAM_H__
#define __INCLUDE_CORE_HRAM_H__

#include <stdint.h>

void coreHramReset(void);
uint8_t coreHramRead(uint16_t p_address);
uint16_t coreHramWrite(uint16_t p_address, uint8_t p_value);

#endif
