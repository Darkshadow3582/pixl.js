#include "db_header.h"

/* Stub of the generated amiibo database: treat every id as unknown.
 * is_valid_amiibo_ntag still accepts ids where head/tail are non-zero. */
const db_amiibo_t *get_amiibo_by_id(uint32_t head, uint32_t tail) {
    (void)head;
    (void)tail;
    return NULL;
}

bool is_valid_amiibo_v3(uint32_t head, uint32_t tail) {
    (void)tail;
    return head == 0xDEADBEEF;
}
