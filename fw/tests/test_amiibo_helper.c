#include "test_framework.h"
#include "amiibo_helper.h"
#include "nfc3d/amiibo.h"
#include "ntag_store.h"

void amiibo_helper_replace_uuid(uint8_t *buffer, const uint8_t uuid[], bool tag_v3);
void amiibo_helper_replace_password(uint8_t *buffer, const uint8_t uuid[]);

static void make_dummy_key_data(uint8_t *data, size_t size) {
    nfc3d_amiibo_keys keys;
    memset(&keys, 0, sizeof(keys));
    for (size_t i = 0; i < sizeof(keys.data.hmacKey); i++) {
        keys.data.hmacKey[i] = (uint8_t)(i * 7 + 1);
        keys.tag.hmacKey[i] = (uint8_t)(i * 13 + 5);
    }
    strcpy(keys.data.typeString, "unfixed infos");
    strcpy(keys.tag.typeString, "locked secret");
    keys.data.magicBytesSize = 14;
    keys.tag.magicBytesSize = 14;
    for (int i = 0; i < 16; i++) {
        keys.data.magicBytes[i] = (uint8_t)(i * 3);
        keys.tag.magicBytes[i] = (uint8_t)(i * 5 + 2);
    }
    for (size_t i = 0; i < sizeof(keys.data.xorPad); i++) {
        keys.data.xorPad[i] = (uint8_t)(i * 11);
        keys.tag.xorPad[i] = (uint8_t)(i * 17 + 3);
    }
    memset(data, 0, size);
    memcpy(data, &keys, sizeof(keys));
}

static void test_crc16_mcrf4xx(void) {
    TF_CASE("crc16_mcrf4xx matches the standard check value");
    const char *check = "123456789";
    TF_CHECK_EQ(crc16_mcrf4xx((const uint8_t *)check, 9), 0x6F91);
    TF_CHECK_EQ(crc16_mcrf4xx((const uint8_t *)"", 0), 0xFFFF);
}

static void test_to_little_endian_int32(void) {
    TF_CASE("to_little_endian_int32 reads big-endian bytes");
    const uint8_t data[4] = {0x11, 0x22, 0x33, 0x44};
    TF_CHECK_EQ(to_little_endian_int32(data), 0x11223344u);
}

static void test_load_keys(void) {
    TF_CASE("load_keys loads valid keys and rejects invalid data");
    TF_CHECK(!amiibo_helper_is_key_loaded());

    uint8_t key_data[160];
    make_dummy_key_data(key_data, sizeof(key_data));
    TF_CHECK_EQ(amiibo_helper_load_keys(key_data), NRF_SUCCESS);
    TF_CHECK(amiibo_helper_is_key_loaded());

    // corrupted magicBytesSize -> rejected
    make_dummy_key_data(key_data, sizeof(key_data));
    key_data[80 + 31] = 17; // tag.magicBytesSize
    TF_CHECK_EQ(amiibo_helper_load_keys(key_data), NRF_ERROR_INVALID_DATA);
}

static void test_replace_uuid(void) {
    TF_CASE("replace_uuid writes UUID with BCC bytes at 0x1D4");
    uint8_t buffer[520];
    memset(buffer, 0, sizeof(buffer));
    const uint8_t uuid[7] = {0x04, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

    amiibo_helper_replace_uuid(buffer, uuid, false);

    TF_CHECK_EQ(buffer[468], 0x04);
    TF_CHECK_EQ(buffer[469], 0x11);
    TF_CHECK_EQ(buffer[470], 0x22);
    TF_CHECK_EQ(buffer[471], 0x88 ^ 0x04 ^ 0x11 ^ 0x22); // bcc0
    TF_CHECK_EQ(buffer[472], 0x33);
    TF_CHECK_EQ(buffer[473], 0x44);
    TF_CHECK_EQ(buffer[474], 0x55);
    TF_CHECK_EQ(buffer[475], 0x66);
    TF_CHECK_EQ(buffer[0], 0x33 ^ 0x44 ^ 0x55 ^ 0x66); // bcc1

    // v3: raw uuid, no bcc
    memset(buffer, 0, sizeof(buffer));
    amiibo_helper_replace_uuid(buffer, uuid, true);
    TF_CHECK_MEM(&buffer[468], uuid, 7);
    TF_CHECK_EQ(buffer[475 + 1], 0x00);
}

static void test_replace_password(void) {
    TF_CASE("replace_password derives the standard NTAG215 password");
    uint8_t buffer[540];
    memset(buffer, 0, sizeof(buffer));
    const uint8_t uuid[7] = {0x04, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

    amiibo_helper_replace_password(buffer, uuid);

    TF_CHECK_EQ(buffer[532], 0xAA ^ 0x11 ^ 0x33);
    TF_CHECK_EQ(buffer[533], 0x55 ^ 0x22 ^ 0x44);
    TF_CHECK_EQ(buffer[534], 0xAA ^ 0x33 ^ 0x55);
    TF_CHECK_EQ(buffer[535], 0x55 ^ 0x44 ^ 0x66);
}

static void test_generate_and_rand_uuid(void) {
    TF_CASE("generate amiibo then randomize its UUID keeps the tag valid");
    uint8_t key_data[160];
    make_dummy_key_data(key_data, sizeof(key_data));
    TF_CHECK_EQ(amiibo_helper_load_keys(key_data), NRF_SUCCESS);

    ntag_t ntag;
    memset(&ntag, 0, sizeof(ntag));
    TF_CHECK_EQ(amiibo_helper_generate_amiibo(0x01931001, 0x1B3B3C1, &ntag), NRF_SUCCESS);
    TF_CHECK_EQ(ntag.type, NTAG_215);
    TF_CHECK_EQ(ntag.data[0], 0x04);
    // trailing lock/config bytes
    TF_CHECK_EQ(ntag.data[520], 0x01);
    TF_CHECK_EQ(ntag.data[522], 0x0F);
    TF_CHECK_EQ(ntag.data[523], 0xBD);
    TF_CHECK(is_valid_amiibo_ntag(&ntag));

    uint8_t old_uuid[7];
    ntag_store_get_uuid(&ntag, old_uuid);

    TF_CHECK_EQ(amiibo_helper_rand_amiibo_uuid(&ntag), NRF_SUCCESS);
    uint8_t new_uuid[7];
    ntag_store_get_uuid(&ntag, new_uuid);
    TF_CHECK(memcmp(old_uuid, new_uuid, 7) != 0);
    // re-signed tag must still unpack (and BCC stay consistent)
    TF_CHECK(is_valid_amiibo_ntag(&ntag));
    TF_CHECK_EQ(ntag.data[3], ntag.data[0] ^ ntag.data[1] ^ ntag.data[2] ^ 0x88);
    TF_CHECK_EQ(ntag.data[8], ntag.data[4] ^ ntag.data[5] ^ ntag.data[6] ^ ntag.data[7]);
}

static void test_rand_uuid_requires_valid_tag(void) {
    TF_CASE("rand_amiibo_uuid fails on an invalid tag");
    ntag_t ntag;
    memset(&ntag, 0, sizeof(ntag));
    TF_CHECK_EQ(amiibo_helper_rand_amiibo_uuid(&ntag), NRF_ERROR_INVALID_DATA);
}

int main(void) {
    test_crc16_mcrf4xx();
    test_to_little_endian_int32();
    test_load_keys();
    test_replace_uuid();
    test_replace_password();
    test_generate_and_rand_uuid();
    test_rand_uuid_requires_valid_tag();
    TF_MAIN_END();
}
