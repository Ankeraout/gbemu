#ifndef __INCLUDE_CORE_CPU_H__
#define __INCLUDE_CORE_CPU_H__

void coreCpuReset(void);
void coreCpuStep(void);
uint8_t coreCpuRead(uint16_t p_address);
void coreCpuWrite(uint16_t p_address, uint8_t p_value);

#endif
