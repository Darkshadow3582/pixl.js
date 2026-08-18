#include "test_framework.h"
#include "ntag_store.h"

static void test_generate_default(void) {
    TF_CASE("ntag_store_generate fills default NTAG215");
    ntag_t tag;
    memset(&tag, 0, sizeof(tag));
    // ntag_store_generate is static-free exported? it is non-static in the non-INTERNAL section
    extern ret_code_t ntag_store_generate(uint8_t idx, ntag_t *ntag);
    TF_CHECK_EQ(ntag_store_generate(0x5A, &tag), NRF_SUCCESS);

    TF_CHECK_EQ(tag.type, NTAG_215);
    // UID + index in byte 7
    TF_CHECK_EQ(tag.data[7], 0x5A);
    // BCC1 = UID3 ^ UID4 ^ UID5 ^ UID6
    TF_CHECK_EQ(tag.data[8], tag.data[4] ^ tag.data[5] ^ tag.data[6] ^ tag.data[7]);
    // capability container of a NTAG215
    TF_CHECK_EQ(tag.data[12], 0xE1);
    TF_CHECK_EQ(tag.data[13], 0x10);
    TF_CHECK_EQ(tag.data[14], 0x3E);
}

static void test_set_get_uuid_215(void) {
    TF_CASE("set/get uuid with BCC bytes for NTAG_215");
    ntag_t tag;
    extern ret_code_t ntag_store_generate(uint8_t idx, ntag_t *ntag);
    ntag_store_generate(0, &tag);

    uint8_t uuid[7] = {0x04, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    ntag_store_set_uuid(&tag, uuid);

    TF_CHECK_EQ(tag.data[0], 0x04);
    TF_CHECK_EQ(tag.data[1], 0x11);
    TF_CHECK_EQ(tag.data[2], 0x22);
    // BCC0 = 0x88 ^ UID0 ^ UID1 ^ UID2
    TF_CHECK_EQ(tag.data[3], 0x88 ^ 0x04 ^ 0x11 ^ 0x22);
    TF_CHECK_EQ(tag.data[4], 0x33);
    TF_CHECK_EQ(tag.data[5], 0x44);
    TF_CHECK_EQ(tag.data[6], 0x55);
    TF_CHECK_EQ(tag.data[7], 0x66);
    // BCC1
    TF_CHECK_EQ(tag.data[8], 0x33 ^ 0x44 ^ 0x55 ^ 0x66);

    uint8_t out[7] = {0};
    ntag_store_get_uuid(&tag, out);
    TF_CHECK_MEM(out, uuid, 7);
}

static void test_set_uuid_i2c(void) {
    TF_CASE("set uuid for NTAG_I2C_PLUS_2K uses internal bytes");
    ntag_t tag;
    extern ret_code_t ntag_store_generate(uint8_t idx, ntag_t *ntag);
    ntag_store_generate(0, &tag);
    tag.type = NTAG_I2C_PLUS_2K;

    uint8_t uuid[7] = {0x04, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    ntag_store_set_uuid(&tag, uuid);

    TF_CHECK_MEM(tag.data, uuid, 7);
    TF_CHECK_EQ(tag.data[7], 0x00);
    TF_CHECK_EQ(tag.data[8], 0x44);

    uint8_t out[7] = {0};
    ntag_store_get_uuid(&tag, out);
    TF_CHECK_MEM(out, uuid, 7);
}

static void test_new_rand(void) {
    TF_CASE("ntag_store_new_rand generates valid random tag");
    ntag_t tag;
    memset(&tag, 0, sizeof(tag));
    ntag_store_new_rand(&tag);

    TF_CHECK_EQ(tag.type, NTAG_215);
    TF_CHECK_EQ(tag.data[0], 0x04);
    TF_CHECK_EQ(tag.data[3], tag.data[0] ^ tag.data[1] ^ tag.data[2] ^ 0x88);
    TF_CHECK_EQ(tag.data[8], tag.data[4] ^ tag.data[5] ^ tag.data[6] ^ tag.data[7]);
}

static void test_uuid_rand(void) {
    TF_CASE("ntag_store_uuid_rand keeps other data intact");
    ntag_t tag;
    memset(&tag, 0, sizeof(tag));
    tag.type = NTAG_215;
    tag.data[100] = 0xAB;

    TF_CHECK_EQ(ntag_store_uuid_rand(&tag), NRF_SUCCESS);
    TF_CHECK_EQ(tag.data[100], 0xAB);
    TF_CHECK_EQ(tag.data[0], 0x04);
    // BCC still consistent
    TF_CHECK_EQ(tag.data[3], tag.data[0] ^ tag.data[1] ^ tag.data[2] ^ 0x88);
}

int main(void) {
    test_generate_default();
    test_set_get_uuid_215();
    test_set_uuid_i2c();
    test_new_rand();
    test_uuid_rand();
    TF_MAIN_END();
}
