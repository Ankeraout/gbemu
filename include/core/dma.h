#ifndef __INCLUDE_CORE_DMA_H__
#define __INCLUDE_CORE_DMA_H__

#include <stdbool.h>
#include <stdint.h>

void coreDmaReset(void);
void coreDmaCycle(void);
uint8_t coreDmaRead(uint16_t p_address);
void coreDmaWrite(uint16_t p_address, uint8_t p_value);
bool coreDmaIsRunning(void);

#endif
