#include "test_framework.h"
#include "ntag_emu.h"

#include "hal_nfc_t2t_mock.h"

static ntag_event_type_t s_last_event;
static int s_event_count;
static ntag_t *s_event_tag;

static void update_cb(ntag_event_type_t type, void *context, ntag_t *ntag) {
    (void)context;
    s_last_event = type;
    s_event_count++;
    s_event_tag = ntag;
}

static ntag_t make_tag(ntag_type_t type) {
    ntag_t tag;
    memset(&tag, 0, sizeof(tag));
    tag.type = type;
    for (size_t i = 0; i < sizeof(tag.data); i++) {
        tag.data[i] = (uint8_t)(i & 0xFF);
    }
    return tag;
}

static void test_init_sets_nfcid1(void) {
    TF_CASE("ntag_emu_init sets 7-byte NFCID1 without BCC bytes");
    hal_mock_reset();
    ntag_t tag = make_tag(NTAG_215);
    uint8_t uuid[7] = {0x04, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    extern void ntag_store_set_uuid(ntag_t *ntag, uint8_t *uuid);
    ntag_store_set_uuid(&tag, uuid);

    TF_CHECK_EQ(ntag_emu_init(&tag), NRF_SUCCESS);
    TF_CHECK_EQ(hal_mock_nfcid1_len, 7);
    // skips BCC0 (data[3]) and BCC1 (data[8])
    const uint8_t expected[7] = {0x04, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    TF_CHECK_MEM(hal_mock_nfcid1, expected, 7);
}

static void test_read_command(void) {
    TF_CASE("READ command returns 16 bytes starting at block");
    hal_mock_reset();
    ntag_t tag = make_tag(NTAG_215);
    ntag_emu_init(&tag);

    uint8_t frame[] = {0x30, 0x04};
    hal_mock_send_command(frame, sizeof(frame));

    TF_CHECK_EQ(hal_mock_last_tx_len, 16);
    TF_CHECK_MEM(hal_mock_last_tx, &ntag_emu_get_current_tag()->data[4 * 4], 16);
}

static void test_read_out_of_range(void) {
    TF_CASE("READ beyond tag size is NAKed");
    hal_mock_reset();
    ntag_t tag = make_tag(NTAG_215);
    ntag_emu_init(&tag);

    uint8_t frame[] = {0x30, 0xFF};
    hal_mock_send_command(frame, sizeof(frame));

    TF_CHECK_EQ(hal_mock_tx_count, 0);
    TF_CHECK_EQ(hal_mock_ack_nack_count, 1);
    TF_CHECK_EQ(hal_mock_last_ack_nack, 0x0);
}

static void test_write_command(void) {
    TF_CASE("WRITE command stores page data and ACKs");
    hal_mock_reset();
    ntag_t tag = make_tag(NTAG_215);
    ntag_emu_init(&tag);

    uint8_t frame[] = {0xA2, 0x10, 0xDE, 0xAD, 0xBE, 0xEF};
    hal_mock_send_command(frame, sizeof(frame));

    ntag_t *cur = ntag_emu_get_current_tag();
    TF_CHECK_EQ(hal_mock_ack_nack_count, 1);
    TF_CHECK_EQ(hal_mock_last_ack_nack, 0xA);
    TF_CHECK_MEM(&cur->data[0x10 * 4], frame + 2, 4);
}

static void test_write_protected_pages(void) {
    TF_CASE("WRITE to dynamic lock pages 133/134 is ignored but ACKed");
    hal_mock_reset();
    ntag_t tag = make_tag(NTAG_215);
    ntag_emu_init(&tag);
    uint8_t before133[8];
    memcpy(before133, &ntag_emu_get_current_tag()->data[133 * 4], 8);

    uint8_t frame[] = {0xA2, 133, 0xDE, 0xAD, 0xBE, 0xEF};
    hal_mock_send_command(frame, sizeof(frame));
    uint8_t frame2[] = {0xA2, 134, 0x01, 0x02, 0x03, 0x04};
    hal_mock_send_command(frame2, sizeof(frame2));

    TF_CHECK_MEM(&ntag_emu_get_current_tag()->data[133 * 4], before133, 8);
    TF_CHECK_EQ(hal_mock_last_ack_nack, 0xA);
}

static void test_write_read_only(void) {
    TF_CASE("WRITE on read-only tag is NAKed");
    hal_mock_reset();
    ntag_t tag = make_tag(NTAG_215);
    tag.read_only = true;
    ntag_emu_init(&tag);

    uint8_t frame[] = {0xA2, 0x10, 0xDE, 0xAD, 0xBE, 0xEF};
    hal_mock_send_command(frame, sizeof(frame));

    TF_CHECK_EQ(hal_mock_last_ack_nack, 0x0);
    TF_CHECK_EQ(ntag_emu_get_current_tag()->data[0x10 * 4], 0x40); // unchanged
}

static void test_write_wrong_length(void) {
    TF_CASE("WRITE with wrong frame length is NAKed");
    hal_mock_reset();
    ntag_t tag = make_tag(NTAG_215);
    ntag_emu_init(&tag);

    uint8_t frame[] = {0xA2, 0x10, 0xDE, 0xAD};
    hal_mock_send_command(frame, sizeof(frame));

    TF_CHECK_EQ(hal_mock_last_ack_nack, 0x0);
}

static void test_get_version(void) {
    TF_CASE("GET_VERSION returns NTAG215 / I2C 2K version info");
    hal_mock_reset();
    ntag_t tag = make_tag(NTAG_215);
    ntag_emu_init(&tag);
    uint8_t frame[] = {0x60};
    hal_mock_send_command(frame, sizeof(frame));
    const uint8_t v215[8] = {0x00, 0x04, 0x04, 0x02, 0x01, 0x00, 0x11, 0x03};
    TF_CHECK_EQ(hal_mock_last_tx_len, 8);
    TF_CHECK_MEM(hal_mock_last_tx, v215, 8);

    hal_mock_reset();
    ntag_t tag2 = make_tag(NTAG_I2C_PLUS_2K);
    ntag_emu_init(&tag2);
    hal_mock_send_command(frame, sizeof(frame));
    const uint8_t v2k[8] = {0x00, 0x04, 0x04, 0x05, 0x02, 0x02, 0x15, 0x03};
    TF_CHECK_EQ(hal_mock_last_tx_len, 8);
    TF_CHECK_MEM(hal_mock_last_tx, v2k, 8);
}

static void test_read_sig_and_pwd(void) {
    TF_CASE("READ_SIG returns 32 bytes, PWD_AUTH returns PwdOK");
    hal_mock_reset();
    ntag_t tag = make_tag(NTAG_215);
    ntag_emu_init(&tag);

    uint8_t sig_frame[] = {0x3C, 0x00};
    hal_mock_send_command(sig_frame, sizeof(sig_frame));
    TF_CHECK_EQ(hal_mock_last_tx_len, 32);

    uint8_t pwd_frame[] = {0x1B, 0x11, 0x22, 0x33, 0x44};
    hal_mock_send_command(pwd_frame, sizeof(pwd_frame));
    TF_CHECK_EQ(hal_mock_last_tx_len, 2);
    TF_CHECK_EQ(hal_mock_last_tx[0], 0x80);
    TF_CHECK_EQ(hal_mock_last_tx[1], 0x80);
}

static void test_fast_read(void) {
    TF_CASE("FAST_READ returns requested page range");
    hal_mock_reset();
    ntag_t tag = make_tag(NTAG_215);
    ntag_emu_init(&tag);

    uint8_t frame[] = {0x3A, 0x00, 0x01}; // pages 0..1
    hal_mock_send_command(frame, sizeof(frame));

    TF_CHECK_EQ(hal_mock_last_tx_len, 8);
    TF_CHECK_MEM(hal_mock_last_tx, ntag_emu_get_current_tag()->data, 8);
}

static void test_sector_select(void) {
    TF_CASE("SECTOR_SELECT switches sector then READ addresses sector 1");
    hal_mock_reset();
    ntag_t tag = make_tag(NTAG_215);
    ntag_emu_init(&tag);

    uint8_t sel1[] = {0xC2, 0xFF};
    hal_mock_send_command(sel1, sizeof(sel1));
    TF_CHECK_EQ(hal_mock_last_ack_nack, 0xA);

    uint8_t sel2[] = {0x01}; // sector 1
    hal_mock_send_command(sel2, sizeof(sel2));
    TF_CHECK_EQ(hal_mock_passive_ack_count, 1);

    // block 0 in sector 1 = block 256, out of range for NTAG215 -> NAK
    hal_mock_reset();
    uint8_t read[] = {0x30, 0x00};
    hal_mock_send_command(read, sizeof(read));
    TF_CHECK_EQ(hal_mock_ack_nack_count, 1);

    // but valid for I2C 2K tag
    hal_mock_reset();
    ntag_t tag2 = make_tag(NTAG_I2C_PLUS_2K);
    ntag_emu_init(&tag2);
    hal_mock_send_command(sel1, sizeof(sel1));
    hal_mock_send_command(sel2, sizeof(sel2));
    hal_mock_reset();
    hal_mock_send_command(read, sizeof(read));
    TF_CHECK_EQ(hal_mock_last_tx_len, 16);
    TF_CHECK_MEM(hal_mock_last_tx, &ntag_emu_get_current_tag()->data[256 * 4], 16);
}

static void test_field_off_events(void) {
    TF_CASE("FIELD_OFF reports WRITTEN when dirty, READ otherwise");
    hal_mock_reset();
    s_event_count = 0;
    ntag_t tag = make_tag(NTAG_215);
    ntag_emu_init(&tag);
    ntag_emu_set_update_cb(update_cb, NULL);

    // no write happened -> READ event
    hal_mock_fire_event(HAL_NFC_EVENT_FIELD_OFF, NULL, 0);
    TF_CHECK_EQ(s_event_count, 1);
    TF_CHECK(s_last_event == NTAG_EVENT_TYPE_READ);

    // now write a page -> dirty -> WRITTEN event
    uint8_t frame[] = {0xA2, 0x10, 0xDE, 0xAD, 0xBE, 0xEF};
    hal_mock_send_command(frame, sizeof(frame));
    hal_mock_fire_event(HAL_NFC_EVENT_FIELD_OFF, NULL, 0);
    TF_CHECK_EQ(s_event_count, 2);
    TF_CHECK(s_last_event == NTAG_EVENT_TYPE_WRITTEN);
    TF_CHECK(s_event_tag == ntag_emu_get_current_tag());

    // event handling clears the dirty flag
    hal_mock_fire_event(HAL_NFC_EVENT_FIELD_OFF, NULL, 0);
    TF_CHECK_EQ(s_event_count, 3);
    TF_CHECK(s_last_event == NTAG_EVENT_TYPE_READ);
}

static void test_set_tag_resets_state(void) {
    TF_CASE("set_tag resets sector select state");
    hal_mock_reset();
    ntag_t tag = make_tag(NTAG_215);
    ntag_emu_init(&tag);

    uint8_t sel1[] = {0xC2, 0xFF};
    hal_mock_send_command(sel1, sizeof(sel1));
    uint8_t sel2[] = {0x01};
    hal_mock_send_command(sel2, sizeof(sel2));

    // re-set the tag: sector must be reset to 0
    ntag_t tag2 = make_tag(NTAG_215);
    ntag_emu_set_tag(&tag2);
    hal_mock_reset();
    uint8_t read[] = {0x30, 0x00};
    hal_mock_send_command(read, sizeof(read));
    TF_CHECK_EQ(hal_mock_last_tx_len, 16);
    TF_CHECK_MEM(hal_mock_last_tx, ntag_emu_get_current_tag()->data, 16);
}

int main(void) {
    test_init_sets_nfcid1();
    test_read_command();
    test_read_out_of_range();
    test_write_command();
    test_write_protected_pages();
    test_write_read_only();
    test_write_wrong_length();
    test_get_version();
    test_read_sig_and_pwd();
    test_fast_read();
    test_sector_select();
    test_field_off_events();
    test_set_tag_resets_state();
    TF_MAIN_END();
}
