/*
 * Host-side mock of the NFC T2T HAL. Captures outgoing frames and lets
 * tests inject reader-side NFC events (field on/off, commands).
 */
#include "hal_nfc_t2t.h"

#include <string.h>

static hal_nfc_callback_t s_callback = NULL;
static void *s_callback_context = NULL;

uint8_t hal_mock_last_tx[512];
size_t hal_mock_last_tx_len = 0;
int hal_mock_tx_count = 0;
uint8_t hal_mock_last_ack_nack = 0;
int hal_mock_ack_nack_count = 0;
int hal_mock_passive_ack_count = 0;
uint8_t hal_mock_nfcid1[10];
size_t hal_mock_nfcid1_len = 0;

void hal_mock_reset(void) {
    /* keeps the registered callback, only clears captured data */
    hal_mock_last_tx_len = 0;
    hal_mock_tx_count = 0;
    hal_mock_ack_nack_count = 0;
    hal_mock_passive_ack_count = 0;
    hal_mock_nfcid1_len = 0;
}

void hal_mock_fire_event(hal_nfc_event_t event, const uint8_t *p_data, size_t data_length) {
    if (s_callback) {
        s_callback(s_callback_context, event, p_data, data_length);
    }
}

void hal_mock_send_command(const uint8_t *frame, size_t len) {
    hal_mock_fire_event(HAL_NFC_EVENT_COMMAND, frame, len);
}

ret_code_t hal_nfc_setup(hal_nfc_callback_t callback, void *p_context) {
    s_callback = callback;
    s_callback_context = p_context;
    return NRF_SUCCESS;
}

ret_code_t hal_nfc_parameter_set(hal_nfc_param_id_t id, void *p_data, size_t data_length) {
    (void)id;
    if (data_length <= sizeof(hal_mock_nfcid1)) {
        memcpy(hal_mock_nfcid1, p_data, data_length);
        hal_mock_nfcid1_len = data_length;
    }
    return NRF_SUCCESS;
}

ret_code_t hal_nfc_parameter_get(hal_nfc_param_id_t id, void *p_data, size_t *p_max_data_length) {
    (void)id;
    (void)p_data;
    (void)p_max_data_length;
    return NRF_SUCCESS;
}

ret_code_t hal_nfc_start(void) { return NRF_SUCCESS; }

ret_code_t hal_nfc_send(const uint8_t *p_data, size_t data_length) {
    if (data_length <= sizeof(hal_mock_last_tx)) {
        memcpy(hal_mock_last_tx, p_data, data_length);
        hal_mock_last_tx_len = data_length;
    }
    hal_mock_tx_count++;
    return NRF_SUCCESS;
}

ret_code_t hal_nfc_stop(void) { return NRF_SUCCESS; }

ret_code_t hal_nfc_done(void) { return NRF_SUCCESS; }

ret_code_t hal_send_ack_nack(uint8_t ack_nack_code) {
    hal_mock_last_ack_nack = ack_nack_code;
    hal_mock_ack_nack_count++;
    return NRF_SUCCESS;
}

void hal_nfc_set_nrfx_irq_enable(uint8_t nrfx_irq_enable) { (void)nrfx_irq_enable; }

ret_code_t hal_nfc_passive_ack(void) {
    hal_mock_passive_ack_count++;
    return NRF_SUCCESS;
}
