#ifndef __INCLUDE_CORE_APU_H__
#define __INCLUDE_CORE_APU_H__

#include <stdint.h>

void coreApuReset(void);
uint8_t coreApuReadIo(uint16_t p_address);
uint8_t coreApuReadWavePattern(uint16_t p_address);
void coreApuWriteIo(uint16_t p_address, uint8_t p_value);
void coreApuWriteWavePattern(uint16_t p_address, uint8_t p_value);

#endif
