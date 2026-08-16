#ifndef AMIIDB_DATA_H
#define AMIIDB_DATA_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * The amiibo database is stored in a packed, string-interned format to save
 * flash:
 *  - name/note strings live in two shared pools (db_name_pool, db_note_pool),
 *    entries are referenced by pool index instead of a 4-byte pointer;
 *  - link entries reference the amiibo by index into amiibo_list instead of
 *    storing a duplicate (head, tail).
 * Use the db_*_get_*() accessors below; do not access pool indices directly.
 */

typedef struct {
    uint32_t head;
    uint32_t tail;
    uint16_t name_en;   /* index into db_name_pool */
    uint16_t name_cn;   /* index into db_name_pool */
} db_amiibo_t;          /* 12 bytes (was 16) */

typedef struct _db_game_t {
    uint8_t game_id;
    uint8_t parent_game_id;
    uint8_t order;
    uint8_t reserved;
    uint16_t link_cnt;
    uint16_t name_en;   /* index into db_name_pool */
    uint16_t name_cn;   /* index into db_name_pool */
} db_game_t;            /* 10 bytes (was 16) */

typedef struct _db_link_t {
    uint16_t amiibo_idx; /* index into amiibo_list */
    uint8_t  game_id;
    uint8_t  note_en;    /* index into db_note_pool */
    uint8_t  note_cn;    /* index into db_note_pool */
    uint8_t  note_it;    /* index into db_note_pool */
} db_link_t;            /* 6 bytes (was 24) */

typedef struct _db_v3_t {
    uint32_t head;
    uint32_t tail;
} db_v3_t;

extern const char db_name_pool[];
extern const uint16_t db_name_pool_offsets[];
extern const char db_note_pool[];
extern const uint16_t db_note_pool_offsets[];

extern const db_amiibo_t amiibo_list[];
extern const size_t amiibo_list_size;
extern const db_game_t game_list[];
extern const db_link_t link_list[];

const db_amiibo_t * get_amiibo_by_id(uint32_t head, uint32_t tail);
const db_link_t* get_link_by_id(uint8_t game_id, uint32_t head, uint32_t tail);
bool is_valid_amiibo_v3(uint32_t head, uint32_t tail);

const char* get_amiibo_display_name(const db_amiibo_t *amiibo);

/* ---- accessors (pool lookups) ---- */

static inline const char *db_amiibo_get_name_en(const db_amiibo_t *amiibo) {
    return &db_name_pool[db_name_pool_offsets[amiibo->name_en]];
}

static inline const char *db_amiibo_get_name_cn(const db_amiibo_t *amiibo) {
    return &db_name_pool[db_name_pool_offsets[amiibo->name_cn]];
}

static inline const char *db_game_get_name_en(const db_game_t *game) {
    return &db_name_pool[db_name_pool_offsets[game->name_en]];
}

static inline const char *db_game_get_name_cn(const db_game_t *game) {
    return &db_name_pool[db_name_pool_offsets[game->name_cn]];
}

static inline const db_amiibo_t *db_link_get_amiibo(const db_link_t *link) {
    return &amiibo_list[link->amiibo_idx];
}

static inline uint32_t db_link_get_head(const db_link_t *link) {
    return amiibo_list[link->amiibo_idx].head;
}

static inline uint32_t db_link_get_tail(const db_link_t *link) {
    return amiibo_list[link->amiibo_idx].tail;
}

static inline const char *db_link_get_note_en(const db_link_t *link) {
    return &db_note_pool[db_note_pool_offsets[link->note_en]];
}

static inline const char *db_link_get_note_cn(const db_link_t *link) {
    return &db_note_pool[db_note_pool_offsets[link->note_cn]];
}

static inline const char *db_link_get_note_it(const db_link_t *link) {
    return &db_note_pool[db_note_pool_offsets[link->note_it]];
}

#endif
