#ifndef __INCLUDE_CORE_DMA_H__
#define __INCLUDE_CORE_DMA_H__

#include <stdint.h>

void coreDmaReset(void);
uint8_t coreDmaRead(uint16_t p_address);
void coreDmaWrite(uint16_t p_address, uint8_t p_value);

#endif
