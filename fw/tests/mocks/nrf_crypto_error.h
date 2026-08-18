#ifndef NRF_CRYPTO_ERROR_MOCK_H_
#define NRF_CRYPTO_ERROR_MOCK_H_

static inline const char *nrf_crypto_error_string_get(ret_code_t error) {
    (void)error;
    return "mock error";
}

#endif
