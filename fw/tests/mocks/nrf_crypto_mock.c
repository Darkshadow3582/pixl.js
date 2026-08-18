/*
 * OpenSSL-backed implementation of the nrf_crypto subset used by amiitool.
 */
#include "nrf_crypto.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <string.h>

#define OPENSSL_SUPPRESS_DEPRECATED

const nrf_crypto_hmac_info_t g_nrf_crypto_hmac_sha256_info = {0};
const nrf_crypto_aes_info_t g_nrf_crypto_aes_ctr_128_info = {0};

static void hmac_ctx_free(nrf_crypto_hmac_context_t *ctx) {
    if (ctx->evp_ctx) {
        EVP_MD_CTX_free((EVP_MD_CTX *)ctx->evp_ctx);
        ctx->evp_ctx = NULL;
    }
}

ret_code_t nrf_crypto_hmac_init(nrf_crypto_hmac_context_t *const p_context,
                                nrf_crypto_hmac_info_t const *p_info,
                                uint8_t const *p_key, size_t key_size) {
    (void)p_info;
    /* Match SDK semantics: init unconditionally re-initializes the context,
     * never touching/freeing whatever was in it before. */
    EVP_PKEY *key = EVP_PKEY_new_raw_private_key(EVP_PKEY_HMAC, NULL, p_key, key_size);
    if (!key) {
        return NRF_ERROR_INVALID_DATA;
    }
    EVP_MD_CTX *md = EVP_MD_CTX_new();
    if (!md || EVP_DigestSignInit(md, NULL, EVP_sha256(), NULL, key) != 1) {
        EVP_PKEY_free(key);
        EVP_MD_CTX_free(md);
        return NRF_ERROR_INVALID_DATA;
    }
    EVP_PKEY_free(key); /* DigestSignInit keeps its own reference */
    p_context->evp_ctx = md;
    return NRF_SUCCESS;
}

ret_code_t nrf_crypto_hmac_update(nrf_crypto_hmac_context_t *const p_context,
                                  uint8_t const *p_data, size_t data_size) {
    if (!p_context->evp_ctx) {
        return NRF_ERROR_INVALID_DATA;
    }
    if (EVP_DigestSignUpdate(p_context->evp_ctx, p_data, data_size) != 1) {
        return NRF_ERROR_INVALID_DATA;
    }
    return NRF_SUCCESS;
}

ret_code_t nrf_crypto_hmac_finalize(nrf_crypto_hmac_context_t *const p_context,
                                    uint8_t *p_digest, size_t *const p_digest_size) {
    if (!p_context->evp_ctx) {
        return NRF_ERROR_INVALID_DATA;
    }
    size_t len = *p_digest_size;
    if (EVP_DigestSignFinal(p_context->evp_ctx, p_digest, &len) != 1) {
        return NRF_ERROR_INVALID_DATA;
    }
    *p_digest_size = len;
    hmac_ctx_free(p_context);
    return NRF_SUCCESS;
}

ret_code_t nrf_crypto_hmac_calculate(nrf_crypto_hmac_context_t *const p_context,
                                     nrf_crypto_hmac_info_t const *p_info,
                                     uint8_t *p_digest, size_t *const p_digest_size,
                                     uint8_t const *p_key, size_t key_size,
                                     uint8_t const *p_data, size_t data_size) {
    ret_code_t err = nrf_crypto_hmac_init(p_context, p_info, p_key, key_size);
    if (err != NRF_SUCCESS) {
        return err;
    }
    err = nrf_crypto_hmac_update(p_context, p_data, data_size);
    if (err != NRF_SUCCESS) {
        return err;
    }
    return nrf_crypto_hmac_finalize(p_context, p_digest, p_digest_size);
}

ret_code_t nrf_crypto_aes_init(nrf_crypto_aes_context_t *const p_context,
                               nrf_crypto_aes_info_t const *p_info,
                               nrf_crypto_operation_t operation) {
    (void)p_info;
    /* CTR mode uses the same operation for both directions */
    (void)operation;
    /* SDK semantics: init unconditionally re-initializes, never frees
     * whatever garbage was in the (often stack-allocated) context. */
    memset(p_context->key, 0, sizeof(p_context->key));
    memset(p_context->iv, 0, sizeof(p_context->iv));
    p_context->has_key = 0;
    p_context->has_iv = 0;
    p_context->operation = (int)operation;
    return NRF_SUCCESS;
}

ret_code_t nrf_crypto_aes_key_set(nrf_crypto_aes_context_t *const p_context, uint8_t *p_key) {
    memcpy(p_context->key, p_key, sizeof(p_context->key));
    p_context->has_key = 1;
    return NRF_SUCCESS;
}

ret_code_t nrf_crypto_aes_iv_set(nrf_crypto_aes_context_t *const p_context, uint8_t *p_iv) {
    memcpy(p_context->iv, p_iv, sizeof(p_context->iv));
    p_context->has_iv = 1;
    return NRF_SUCCESS;
}

ret_code_t nrf_crypto_aes_finalize(nrf_crypto_aes_context_t *const p_context,
                                   uint8_t const *p_data_in, size_t data_in_size,
                                   uint8_t *p_data_out, size_t *const p_data_out_size) {
    if (!p_context->has_key || !p_context->has_iv) {
        return NRF_ERROR_INVALID_DATA;
    }
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return NRF_ERROR_INVALID_DATA;
    }
    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_ctr(), NULL, p_context->key, p_context->iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return NRF_ERROR_INVALID_DATA;
    }
    int out_len = 0;
    /* CTR is symmetric: EncryptUpdate also "decrypts" */
    if (EVP_EncryptUpdate(ctx, p_data_out, &out_len, p_data_in, (int)data_in_size) != 1 ||
        (size_t)out_len != data_in_size) {
        EVP_CIPHER_CTX_free(ctx);
        return NRF_ERROR_INVALID_DATA;
    }
    EVP_CIPHER_CTX_free(ctx);
    *p_data_out_size = data_in_size;
    return NRF_SUCCESS;
}
