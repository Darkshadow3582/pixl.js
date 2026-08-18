#ifndef SDK_ERRORS_MOCK_H_
#define SDK_ERRORS_MOCK_H_

#include <stdint.h>

typedef uint32_t ret_code_t;

#define NRF_SUCCESS 0
#ifndef NRF_ERROR_INVALID_DATA
#define NRF_ERROR_INVALID_DATA 0x0C0D
#endif
#define NRF_ERROR_NO_MEM 0x0C04
#define NRF_ERROR_NOT_FOUND 0x0C11
#define NRF_ERROR_TIMEOUT 0x0C18

#endif
