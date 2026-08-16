//
// Created by solos on 8/20/2023.
//

#include "amiidb_api_db.h"
#include "db_header.h"
#include <string.h>

extern char *strcasestr(const char *, const char *);

int32_t amiidb_api_db_search(const char *search, amiidb_db_search_cb_t cb, void *ctx) {
    const db_amiibo_t *p_amiibo = amiibo_list;
    for (size_t i = 0; i < amiibo_list_size; i++, p_amiibo++) {
        if (strlen(search) > 0 && strcasestr(db_amiibo_get_name_en(p_amiibo), search) != NULL) {
            cb(p_amiibo, ctx);
        }
    }
    return 0;
}