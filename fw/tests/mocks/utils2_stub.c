#include "utils2.h"

#include <stdlib.h>

/* Deterministic-enough PRNG for tests (simple xorshift) */
static uint32_t s_state = 0x12345678;

ret_code_t utils_rand_bytes(uint8_t rand[], uint8_t bytes) {
    for (int i = 0; i < bytes; i++) {
        s_state ^= s_state << 13;
        s_state ^= s_state >> 17;
        s_state ^= s_state << 5;
        rand[i] = (uint8_t)s_state;
    }
    return NRF_SUCCESS;
}

void int32_to_bytes_le(uint32_t val, uint8_t *data) {
    data[0] = val >> 24;
    data[1] = val >> 16;
    data[2] = val >> 8;
    data[3] = val >> 0;
}
