#include <stdint.h>

#include "common.h"
#include "core/bus.h"

typedef uint8_t (*t_coreBusReadFunc)(uint16_t p_address);
typedef void (*t_coreBusWriteFunc)(uint16_t p_address, uint8_t p_value);

static t_coreBusReadFunc s_coreBusReadFuncTable[65536];
static t_coreBusWriteFunc s_coreBusWriteFuncTable[65536];

static uint8_t coreBusReadOpenBus(uint16_t p_address);
static void coreBusWriteOpenBus(uint16_t p_address, uint8_t p_value);

void coreBusReset(void) {
    for(int i = 0; i < 65536; i++) {
        s_coreBusReadFuncTable[i] = coreBusReadOpenBus;
        s_coreBusWriteFuncTable[i] = coreBusWriteOpenBus;
    }
}

uint8_t coreBusRead(uint16_t p_address) {
    return s_coreBusReadFuncTable[p_address](p_address);
}

void coreBusWrite(uint16_t p_address, uint8_t p_value) {
    s_coreBusWriteFuncTable[p_address](p_address, p_value);
}

void coreBusCycle(void) {

}

static uint8_t coreBusReadOpenBus(uint16_t p_address) {
    M_UNUSED_PARAMETER(p_address);

    return 0xff;
}

static void coreBusWriteOpenBus(uint16_t p_address, uint8_t p_value) {
    M_UNUSED_PARAMETER(p_address);
    M_UNUSED_PARAMETER(p_value);

    // Ignore the write
}
