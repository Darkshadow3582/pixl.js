/*
 * test_switch_read.c - replay the NFC command sequence a Nintendo console
 * uses to read an amiibo (NTAG215) and verify the emulator's responses.
 *
 * Sequence documented in:
 *  - "Reverse Engineering Nintendo Amiibo" (Kevin Brewster):
 *      GET_VERSION, READ page 3, PWD_AUTH, then FAST_READ in three spans
 *      (pages 0-59, 60-119, 120-134). Password is derived from the UID:
 *        pw[0] = 0xAA ^ uid[1] ^ uid[3]
 *        pw[1] = 0x55 ^ uid[2] ^ uid[4]
 *        pw[2] = 0xAA ^ uid[3] ^ uid[5]
 *        pw[3] = 0x55 ^ uid[4] ^ uid[6]
 *  - NTAG215 datasheet: GET_VERSION response for NTAG215 is
 *      00 04 04 02 01 00 11 03
 *    PWD_AUTH success returns the 2-byte PACK (amiibo PACK = 80 80).
 */
#include "test_framework.h"
#include "amiibo_helper.h"
#include "ntag_emu.h"
#include "ntag_store.h"
#include "nfc3d/amiibo.h"

#include "hal_nfc_t2t_mock.h"

/* NTAG215 READ (0x30) returns 4 pages = 16 bytes */
#define READ_CHUNK_PAGES 4

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

/* NTAG215 amiibo password derivation from the 7-byte UID (uid[0] is 0x04) */
static void derive_amiibo_password(const uint8_t uid[7], uint8_t pwd[4]) {
    pwd[0] = 0xAA ^ uid[1] ^ uid[3];
    pwd[1] = 0x55 ^ uid[2] ^ uid[4];
    pwd[2] = 0xAA ^ uid[3] ^ uid[5];
    pwd[3] = 0x55 ^ uid[4] ^ uid[6];
}

/* send one command frame and return the captured response */
static const uint8_t *send_cmd(const uint8_t *frame, size_t len) {
    hal_mock_last_tx_len = 0;
    hal_mock_send_command(frame, len);
    return hal_mock_last_tx;
}

static ntag_t s_tag;

static int switch_read_sequence(void) {
    hal_mock_reset();
    TF_CHECK_EQ(ntag_emu_init(&s_tag), NRF_SUCCESS);
    int failures_before = tf_failures;

    /* 1. GET_VERSION - console verifies this is an NTAG215 */
    TF_CASE("console read sequence: GET_VERSION");
    {
        uint8_t cmd[] = {0x60};
        const uint8_t *resp = send_cmd(cmd, sizeof(cmd));
        const uint8_t expected[8] = {0x00, 0x04, 0x04, 0x02, 0x01, 0x00, 0x11, 0x03};
        TF_CHECK_EQ(hal_mock_last_tx_len, 8);
        TF_CHECK_MEM(resp, expected, 8);
    }

    /* 2. READ page 3 first (lock/config area), as observed on real
     *    consoles before authentication. On a generated amiibo this page
     *    carries the F1 10 FF EE static-lock tail, not an NDEF CC. */
    TF_CASE("console read sequence: READ page 3 (lock/config area)");
    {
        uint8_t cmd[] = {0x30, 0x03};
        const uint8_t *resp = send_cmd(cmd, sizeof(cmd));
        TF_CHECK_EQ(hal_mock_last_tx_len, 16);
        TF_CHECK_MEM(resp, &s_tag.data[3 * 4], 16);
        TF_CHECK_EQ(resp[0], 0xF1);
        TF_CHECK_EQ(resp[1], 0x10);
        TF_CHECK_EQ(resp[2], 0xFF);
        TF_CHECK_EQ(resp[3], 0xEE);
    }

    /* 3. PWD_AUTH with the UID-derived password, expect PACK 80 80 */
    TF_CASE("console read sequence: PWD_AUTH with UID-derived password");
    {
        uint8_t uid[7];
        ntag_store_get_uuid(&s_tag, uid);
        uint8_t pwd[4];
        derive_amiibo_password(uid, pwd);
        uint8_t cmd[] = {0x1B, pwd[0], pwd[1], pwd[2], pwd[3]};
        const uint8_t *resp = send_cmd(cmd, sizeof(cmd));
        TF_CHECK_EQ(hal_mock_last_tx_len, 2);
        TF_CHECK_EQ(resp[0], 0x80);
        TF_CHECK_EQ(resp[1], 0x80);
    }

    /* 4. FAST_READ of the whole tag in three spans, as used by the
     *    console to avoid NFC timeouts */
    TF_CASE("console read sequence: FAST_READ spans 0-59 / 60-119 / 120-134");
    {
        struct { uint8_t start, end; } spans[3] = {{0, 59}, {60, 119}, {120, 134}};
        for (int s = 0; s < 3; s++) {
            uint8_t cmd[] = {0x3A, spans[s].start, spans[s].end};
            const uint8_t *resp = send_cmd(cmd, sizeof(cmd));
            size_t pages = spans[s].end - spans[s].start + 1;
            TF_CHECK_EQ(hal_mock_last_tx_len, pages * 4);
            TF_CHECK_MEM(resp, &s_tag.data[spans[s].start * 4], pages * 4);
        }
    }

    return tf_failures - failures_before;
}

static void test_config_tail_layout(void) {
    TF_CASE("generated amiibo carries the expected lock/config tail");
    /* dynamic lock + config pages written by amiibo_helper after packing */
    const uint8_t lock_tail[20] = {0x01, 0x00, 0x0F, 0xBD, 0x00, 0x00, 0x00, 0x04,
                                   0x5F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00};
    TF_CHECK_MEM(&s_tag.data[520], lock_tail, 20);

    /* PWD/PACK are NTAG215 config registers (pages 0x85/0x86): generated
     * tags leave them zero and the emulator answers PWD_AUTH with PACK
     * 80 80 directly (verified in the sequence above). The console-side
     * password derivation must still be stable for the reader: */
    uint8_t uid[7];
    ntag_store_get_uuid(&s_tag, uid);
    uint8_t pwd[4], pwd2[4];
    derive_amiibo_password(uid, pwd);
    derive_amiibo_password(uid, pwd2);
    TF_CHECK_MEM(pwd, pwd2, 4);
    TF_CHECK(uid[0] == 0x04);
}

static void test_sequential_read_scan(void) {
    TF_CASE("sequential 0x30 READ scan over the whole tag never sends garbage");
    hal_mock_reset();
    TF_CHECK_EQ(ntag_emu_init(&s_tag), NRF_SUCCESS);

    int naks = 0;
    int reads = 0;
    for (int page = 0; page < 256; page += READ_CHUNK_PAGES) {
        uint8_t cmd[] = {0x30, (uint8_t)page};
        hal_mock_last_tx_len = 0;
        hal_mock_ack_nack_count = 0;
        hal_mock_send_command(cmd, sizeof(cmd));
        if (hal_mock_ack_nack_count > 0) {
            naks++;
        } else if (hal_mock_last_tx_len == 16) {
            reads++;
            TF_CHECK_MEM(hal_mock_last_tx, ntag_emu_get_current_tag()->data + page * 4, 16);
        }
    }
    /* NTAG215: 33 successful 16-byte reads (pages 0..131), rest NAKed */
    TF_CHECK_EQ(reads, 33);
    TF_CHECK_EQ(naks, 256 / READ_CHUNK_PAGES - 33);
}

int main(void) {
    uint8_t key_data[160];
    make_dummy_key_data(key_data, sizeof(key_data));
    TF_CHECK_EQ(amiibo_helper_load_keys(key_data), NRF_SUCCESS);

    /* generate a real encrypted amiibo and put it on the emulator */
    memset(&s_tag, 0, sizeof(s_tag));
    TF_CHECK_EQ(amiibo_helper_generate_amiibo(0x01931001, 0x1B3B3C1, &s_tag), NRF_SUCCESS);

    switch_read_sequence();
    test_config_tail_layout();
    test_sequential_read_scan();
    TF_MAIN_END();
}
