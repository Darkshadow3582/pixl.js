#ifndef SDK_MACROS_MOCK_H_
#define SDK_MACROS_MOCK_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "sdk_errors.h"

#define VERIFY_SUCCESS(expr)             \
    do {                                 \
        ret_code_t _err = (expr);        \
        if (_err != NRF_SUCCESS) {       \
            return _err;                 \
        }                                \
    } while (0)

#ifndef APP_ERROR_CHECK
#define APP_ERROR_CHECK(err_code)                                                 \
    do {                                                                          \
        if ((err_code) != NRF_SUCCESS) {                                          \
            fprintf(stderr, "APP_ERROR 0x%x at %s:%d\n", (unsigned)(err_code),    \
                    __FILE__, __LINE__);                                          \
            abort();                                                              \
        }                                                                         \
    } while (0)
#endif

#endif
