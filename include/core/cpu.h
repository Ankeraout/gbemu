#ifndef __INCLUDE_CORE_CPU_H__
#define __INCLUDE_CORE_CPU_H__

#include <stdint.h>

enum te_cpuInterrupt {
    E_CPUINTERRUPT_VBLANK,
    E_CPUINTERRUPT_STAT,
    E_CPUINTERRUPT_TIMER,
    E_CPUINTERRUPT_SERIAL,
    E_CPUINTERRUPT_JOYPAD
};

void coreCpuReset(void);
void coreCpuStep(void);
uint8_t coreCpuRead(uint16_t p_address);
void coreCpuWrite(uint16_t p_address, uint8_t p_value);
void coreCpuRequestInterrupt(enum te_cpuInterrupt p_interrupt);

#endif
