#ifndef APP_ERROR_MOCK_H_
#define APP_ERROR_MOCK_H_

#include <assert.h>
#include "sdk_errors.h"

#define APP_ERROR_CHECK(err_code) assert((err_code) == NRF_SUCCESS)

#endif
