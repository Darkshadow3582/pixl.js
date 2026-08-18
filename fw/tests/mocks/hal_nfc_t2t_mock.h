#ifndef HAL_NFC_T2T_MOCK_H_
#define HAL_NFC_T2T_MOCK_H_

#include <stddef.h>
#include <stdint.h>

#include "hal_nfc_t2t.h"

/* captured state, readable from tests */
extern uint8_t hal_mock_last_tx[128];
extern size_t hal_mock_last_tx_len;
extern int hal_mock_tx_count;
extern uint8_t hal_mock_last_ack_nack;
extern int hal_mock_ack_nack_count;
extern int hal_mock_passive_ack_count;
extern uint8_t hal_mock_nfcid1[10];
extern size_t hal_mock_nfcid1_len;

void hal_mock_reset(void);
void hal_mock_fire_event(hal_nfc_event_t event, const uint8_t *p_data, size_t data_length);
void hal_mock_send_command(const uint8_t *frame, size_t len);

#endif
