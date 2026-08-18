#ifndef NRF_CRYPTO_MOCK_H_
#define NRF_CRYPTO_MOCK_H_

/*
 * Host-side nrf_crypto mock backed by OpenSSL libcrypto.
 * Only the APIs used by components/amiitool are provided:
 *   - HMAC-SHA256 (init/update/finalize/calculate)
 *   - AES-128-CTR (init/key_set/iv_set/finalize)
 */

#include <stddef.h>
#include <stdint.h>

#include "sdk_errors.h"

typedef struct {
    int unused;
} nrf_crypto_hmac_info_t;

typedef struct {
    int unused;
} nrf_crypto_aes_info_t;

typedef struct {
    void *evp_ctx;
    uint8_t key[32];
    size_t key_size;
    int initialized;
} nrf_crypto_hmac_context_t;

typedef struct {
    void *evp_ctx;
    uint8_t key[16];
    uint8_t iv[16];
    int operation;
    int has_key;
    int has_iv;
} nrf_crypto_aes_context_t;

typedef enum {
    NRF_CRYPTO_DECRYPT = 1,
    NRF_CRYPTO_ENCRYPT = 2,
} nrf_crypto_operation_t;

extern const nrf_crypto_hmac_info_t g_nrf_crypto_hmac_sha256_info;
extern const nrf_crypto_aes_info_t g_nrf_crypto_aes_ctr_128_info;

ret_code_t nrf_crypto_hmac_init(nrf_crypto_hmac_context_t *const p_context,
                                nrf_crypto_hmac_info_t const *p_info,
                                uint8_t const *p_key, size_t key_size);

ret_code_t nrf_crypto_hmac_update(nrf_crypto_hmac_context_t *const p_context,
                                  uint8_t const *p_data, size_t data_size);

ret_code_t nrf_crypto_hmac_finalize(nrf_crypto_hmac_context_t *const p_context,
                                    uint8_t *p_digest, size_t *const p_digest_size);

ret_code_t nrf_crypto_hmac_calculate(nrf_crypto_hmac_context_t *const p_context,
                                     nrf_crypto_hmac_info_t const *p_info,
                                     uint8_t *p_digest, size_t *const p_digest_size,
                                     uint8_t const *p_key, size_t key_size,
                                     uint8_t const *p_data, size_t data_size);

ret_code_t nrf_crypto_aes_init(nrf_crypto_aes_context_t *const p_context,
                               nrf_crypto_aes_info_t const *p_info,
                               nrf_crypto_operation_t operation);

ret_code_t nrf_crypto_aes_key_set(nrf_crypto_aes_context_t *const p_context, uint8_t *p_key);

ret_code_t nrf_crypto_aes_iv_set(nrf_crypto_aes_context_t *const p_context, uint8_t *p_iv);

ret_code_t nrf_crypto_aes_finalize(nrf_crypto_aes_context_t *const p_context,
                                   uint8_t const *p_data_in, size_t data_in_size,
                                   uint8_t *p_data_out, size_t *const p_data_out_size);

#endif
