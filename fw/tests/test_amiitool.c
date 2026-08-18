#include "test_framework.h"
#include "nfc3d/amiibo.h"
#include "nfc3d/drbg.h"
#include "nfc3d/keygen.h"

/* Build a deterministic dummy keyset that passes nfc3d_amiibo_load_keys */
static void make_dummy_keys(nfc3d_amiibo_keys *keys) {
    memset(keys, 0, sizeof(*keys));
    for (size_t i = 0; i < sizeof(keys->data.hmacKey); i++) {
        keys->data.hmacKey[i] = (uint8_t)(i * 7 + 1);
        keys->tag.hmacKey[i] = (uint8_t)(i * 13 + 5);
    }
    strcpy(keys->data.typeString, "unfixed infos");
    strcpy(keys->tag.typeString, "locked secret");
    keys->data.magicBytesSize = 14;
    keys->tag.magicBytesSize = 14;
    for (int i = 0; i < 16; i++) {
        keys->data.magicBytes[i] = (uint8_t)(i * 3);
        keys->tag.magicBytes[i] = (uint8_t)(i * 5 + 2);
    }
    for (size_t i = 0; i < sizeof(keys->data.xorPad); i++) {
        keys->data.xorPad[i] = (uint8_t)(i * 11);
        keys->tag.xorPad[i] = (uint8_t)(i * 17 + 3);
    }
}

/* Build a well-formed plain amiibo buffer (520 bytes) */
static void make_plain_amiibo(uint8_t *plain) {
    memset(plain, 0, NFC3D_AMIIBO_SIZE);
    for (size_t i = 0; i < NFC3D_AMIIBO_SIZE; i++) {
        plain[i] = (uint8_t)(i * 31 + 7);
    }
    plain[0] = 0x04; // UID start
}

static void test_drbg_deterministic(void) {
    TF_CASE("drbg is deterministic for identical key/seed");
    uint8_t key[16];
    uint8_t seed[32];
    for (int i = 0; i < 16; i++) key[i] = (uint8_t)i;
    for (int i = 0; i < 32; i++) seed[i] = (uint8_t)(0xA0 + i);

    uint8_t out1[64], out2[64], out3[64];
    nfc3d_drbg_generate_bytes(key, sizeof(key), seed, sizeof(seed), out1, sizeof(out1));
    nfc3d_drbg_generate_bytes(key, sizeof(key), seed, sizeof(seed), out2, sizeof(out2));
    seed[0] ^= 0x01;
    nfc3d_drbg_generate_bytes(key, sizeof(key), seed, sizeof(seed), out3, sizeof(out3));

    TF_CHECK_MEM(out1, out2, sizeof(out1));
    TF_CHECK(memcmp(out1, out3, sizeof(out1)) != 0);
}

static void test_keygen_deterministic(void) {
    TF_CASE("keygen derives stable keys from master keys and seed");
    nfc3d_amiibo_keys keys;
    make_dummy_keys(&keys);
    uint8_t seed[NFC3D_KEYGEN_SEED_SIZE];
    for (size_t i = 0; i < sizeof(seed); i++) seed[i] = (uint8_t)(i ^ 0x5A);

    nfc3d_keygen_derivedkeys d1, d2, d3;
    nfc3d_keygen(&keys.data, seed, &d1);
    nfc3d_keygen(&keys.data, seed, &d2);
    seed[0] ^= 0x01;
    nfc3d_keygen(&keys.data, seed, &d3);

    TF_CHECK_MEM(&d1, &d2, sizeof(d1));
    TF_CHECK(memcmp(&d1, &d3, sizeof(d1)) != 0);
}

static void test_load_keys_validation(void) {
    TF_CASE("load_keys accepts valid keys and rejects oversized magic");
    uint8_t key_data[160];
    nfc3d_amiibo_keys parsed;

    nfc3d_amiibo_keys good;
    make_dummy_keys(&good);
    memcpy(key_data, &good, sizeof(good));
    TF_CHECK(nfc3d_amiibo_load_keys(&parsed, key_data));
    TF_CHECK_EQ(parsed.data.magicBytesSize, 14);

    // magicBytesSize offset within nfc3d_keygen_masterkeys = 16 + 14 + 1 = 31
    nfc3d_amiibo_keys bad;
    make_dummy_keys(&bad);
    bad.tag.magicBytesSize = 17;
    memcpy(key_data, &bad, sizeof(bad));
    TF_CHECK(!nfc3d_amiibo_load_keys(&parsed, key_data));

    TF_CHECK(!nfc3d_amiibo_load_keys(&parsed, NULL));
}

/* compare two plain buffers ignoring the two 32-byte HMAC regions,
 * which unpack overwrites with the freshly computed HMACs */
static int plain_equal_no_hmac(const uint8_t *a, const uint8_t *b) {
    for (size_t i = 0; i < NFC3D_AMIIBO_SIZE; i++) {
        if ((i >= 0x008 && i < 0x008 + 32) || (i >= 0x1B4 && i < 0x1B4 + 32)) {
            continue;
        }
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static void test_pack_unpack_roundtrip(void) {
    TF_CASE("pack then unpack restores the original plain data and HMACs verify");
    nfc3d_amiibo_keys keys;
    make_dummy_keys(&keys);

    uint8_t plain[NFC3D_AMIIBO_SIZE];
    make_plain_amiibo(plain);

    uint8_t tag[NTAG215_SIZE];
    memset(tag, 0, sizeof(tag));
    nfc3d_amiibo_pack(&keys, plain, tag, false);
    // packing must not be a no-op
    TF_CHECK(memcmp(tag, plain, NFC3D_AMIIBO_SIZE) != 0);

    uint8_t plain2[NFC3D_AMIIBO_SIZE];
    memset(plain2, 0, sizeof(plain2));
    TF_CHECK(nfc3d_amiibo_unpack(&keys, tag, plain2, false));
    TF_CHECK(plain_equal_no_hmac(plain2, plain));
}

static void test_tampered_tag_fails_unpack(void) {
    TF_CASE("unpack detects tampered data");
    nfc3d_amiibo_keys keys;
    make_dummy_keys(&keys);

    uint8_t plain[NFC3D_AMIIBO_SIZE];
    make_plain_amiibo(plain);

    uint8_t tag[NTAG215_SIZE];
    memset(tag, 0, sizeof(tag));
    nfc3d_amiibo_pack(&keys, plain, tag, false);

    // flip a byte inside the encrypted app data region
    tag[0x30] ^= 0xFF;

    uint8_t plain2[NFC3D_AMIIBO_SIZE];
    TF_CHECK(!nfc3d_amiibo_unpack(&keys, tag, plain2, false));
}

static void test_v3_roundtrip(void) {
    TF_CASE("v3 (I2C 2K) pack/unpack roundtrip");
    nfc3d_amiibo_keys keys;
    make_dummy_keys(&keys);

    uint8_t plain[NFC3D_AMIIBO_SIZE];
    make_plain_amiibo(plain);

    /* v3 internal_to_tag writes up to offset 0x248, beyond NTAG215_SIZE;
     * the firmware always packs into the larger ntag_t buffer */
    uint8_t tag[1024];
    memset(tag, 0, sizeof(tag));
    nfc3d_amiibo_pack(&keys, plain, tag, true);

    uint8_t plain2[NFC3D_AMIIBO_SIZE];
    memset(plain2, 0, sizeof(plain2));
    TF_CHECK(nfc3d_amiibo_unpack(&keys, tag, plain2, true));
    TF_CHECK(plain_equal_no_hmac(plain2, plain));
}

int main(void) {
    test_drbg_deterministic();
    test_keygen_deterministic();
    test_load_keys_validation();
    test_pack_unpack_roundtrip();
    test_tampered_tag_fails_unpack();
    test_v3_roundtrip();
    TF_MAIN_END();
}
