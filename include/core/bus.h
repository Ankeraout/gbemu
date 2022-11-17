#ifndef __INCLUDE_CORE_BUS_H__
#define __INCLUDE_CORE_BUS_H__

#include <stdint.h>

void coreBusReset(void);
uint8_t coreBusRead(uint16_t p_address);
void coreBusWrite(uint16_t p_address, uint8_t p_value);
void coreBusCycle(void);

#endif
