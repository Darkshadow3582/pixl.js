#ifndef MUI_VIEW_DISPATCHER_H
#define MUI_VIEW_DISPATCHER_H

#include "mui_anim.h"
#include "mui_common.h"
#include "mui_core.h"
#include "mui_view.h"
#include "mui_view_port.h"

typedef enum {
    MUI_TRANSITION_DIR_RIGHT,
    MUI_TRANSITION_DIR_LEFT,
    MUI_TRANSITION_DIR_UP,
    MUI_TRANSITION_DIR_DOWN,
} mui_transition_dir_t;

DICT_DEF2(mui_view_dict, uint32_t, M_DEFAULT_OPLIST, mui_view_t*, M_PTR_OPLIST)

typedef struct {
    mui_view_port_t* p_view_port;
    mui_view_dict_t views;
    mui_view_t* p_active_view;
    mui_transition_dir_t transition_dir;
    mui_anim_t transition_anim;
    uint8_t *p_transition_old_buf;
    uint8_t *p_transition_new_buf;
    bool transition_active;
} mui_view_dispatcher_t;


mui_view_dispatcher_t* mui_view_dispatcher_create();
void mui_view_dispatcher_free(mui_view_dispatcher_t* p_dispatcher);
void mui_view_dispatcher_add_view(mui_view_dispatcher_t* p_dispatcher, uint32_t view_id, mui_view_t* p_view);
void mui_view_dispatcher_remove_view(mui_view_dispatcher_t* p_dispatcher, uint32_t view_id);
void mui_view_dispatcher_attach(mui_view_dispatcher_t* p_dispatcher, mui_layer_t layer);
void mui_view_dispatcher_detach(mui_view_dispatcher_t* p_dispatcher, mui_layer_t layer);
void mui_view_dispatcher_switch_to_view(mui_view_dispatcher_t* p_dispatcher, uint32_t view_id);
void mui_view_dispatcher_set_transition_dir(mui_view_dispatcher_t *p_dispatcher, mui_transition_dir_t dir);


#endif