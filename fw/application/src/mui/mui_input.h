#ifndef MUI_INPUT_H
#define MUI_INPUT_H

#include "mui_common.h"
#include "bsp.h"


typedef enum {
    INPUT_KEY_LEFT,
    INPUT_KEY_CENTER,
    INPUT_KEY_RIGHT,
    INPUT_KEY_BACK,
    INPUT_KEY_MAX
} input_key_t;

typedef enum {
    INPUT_TYPE_PRESS,
    INPUT_TYPE_RELEASE,
    INPUT_TYPE_SHORT,
    INPUT_TYPE_LONG,
    INPUT_TYPE_REPEAT

} input_type_t;


typedef struct {
    input_key_t key;
    input_type_t type;
} mui_input_event_t;

typedef void (*mui_input_return_key_detect_cb_t)(bool detected, uint8_t pin, void *user_data);

void mui_input_init();

void mui_input_on_bsp_event(bsp_event_t evt);
bool mui_input_return_key_detect_start(mui_input_return_key_detect_cb_t cb, void *user_data);
void mui_input_return_key_detect_cancel();
bool mui_input_return_key_detect_is_active();

#endif
