#include "db_header.h"

#include "settings.h"

const db_amiibo_t * get_amiibo_by_id(uint32_t head, uint32_t tail){
    int left = 0;
    int right = amiibo_list_size - 1;
    while (left <= right) {
        int mid_index = (left + right) / 2;
        const db_amiibo_t *mid = &amiibo_list[mid_index];

        // Compare by (head, tail)
        if (mid->head < head || (mid->head == head && mid->tail < tail)) {
            left = mid_index + 1;
        } else if (mid->head == head && mid->tail == tail) {
            return mid;
        } else {
            right = mid_index - 1;
        }
    }
    return 0;
}

const db_link_t* get_link_by_id(uint8_t game_id, uint32_t head, uint32_t tail){
    // Resolve the amiibo to its index in amiibo_list first; links reference
    // amiibo by index instead of storing a duplicate (head, tail).
    const db_amiibo_t *amiibo = get_amiibo_by_id(head, tail);
    if (amiibo == 0) {
        return 0;
    }
    uint16_t amiibo_idx = (uint16_t)(amiibo - amiibo_list);

    const db_link_t *link = link_list;
    while(link->game_id > 0){
        if(link->game_id == game_id && link->amiibo_idx == amiibo_idx){
            return link;
        }
        link += 1;
    }
    return 0;
}

bool is_valid_amiibo_v3(uint32_t head, uint32_t tail){
    return (tail & 0xFF) == 0x03;
}

const char* get_amiibo_display_name(const db_amiibo_t *amiibo){
     uint8_t language = settings_get_data()->language;
     const char *name_cn = db_amiibo_get_name_cn(amiibo);
    return language == LANGUAGE_ZH_HANS  && name_cn[0] != '\0' ? name_cn : db_amiibo_get_name_en(amiibo);
}
