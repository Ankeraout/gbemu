#ifndef __INCLUDE_CORE_TIMER_H__
#define __INCLUDE_CORE_TIMER_H__

#include <stdint.h>

void coreTimerReset(void);
uint8_t coreTimerRead(uint16_t p_address);
void coreTimerWrite(uint16_t p_address, uint8_t p_value);

#endif
