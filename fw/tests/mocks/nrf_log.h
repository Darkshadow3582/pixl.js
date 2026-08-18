#ifndef NRF_LOG_MOCK_H_
#define NRF_LOG_MOCK_H_

/* Host-side no-op logging stubs.
 * The SDK's nrf_log.h transitively provides string.h and sdk_macros.h
 * (VERIFY_SUCCESS etc.), so the mock provides them too. */

#include <stdio.h>
#include <string.h>
#include "sdk_macros.h"

#define NRF_LOG_ERROR(...) ((void)0)
#define NRF_LOG_WARNING(...) ((void)0)
#define NRF_LOG_INFO(...) ((void)0)
#define NRF_LOG_DEBUG(...) ((void)0)
#define NRF_LOG_RAW_INFO(...) ((void)0)
#define NRF_LOG_RAW_DEBUG(...) ((void)0)

#endif
