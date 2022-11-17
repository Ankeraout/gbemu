#ifndef __INCLUDE_CORE_WRAM_H__
#define __INCLUDE_CORE_WRAM_H__

#include <stdint.h>

void coreWramReset(void);
uint8_t coreWramRead(uint16_t p_address);
void coreWramWrite(uint16_t p_address, uint8_t p_value);

#endif
