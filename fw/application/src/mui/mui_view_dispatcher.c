#include "mui_view_dispatcher.h"
#include "mui_core.h"
#include "mui_anim.h"
#include "settings.h"

static void mui_view_dispatcher_transition_exec_cb(void *var, int32_t value) {
    (void)value;
    mui_view_dispatcher_t *p_d = (mui_view_dispatcher_t *)var;
    mui_view_port_update(p_d->p_view_port);
}

static void mui_view_dispatcher_draw_transition(mui_view_dispatcher_t *p_d, mui_canvas_t *p_canvas) {
    u8g2_t *u8g2 = p_canvas->fb;
    uint8_t *fb = u8g2_GetBufferPtr(u8g2);
    int32_t t = p_d->transition_anim.current_value;
    int32_t sw = (int32_t)mui_canvas_get_width(p_canvas);

    // First time: let new view draw and capture its buffer
    if (!p_d->p_transition_new_buf) {
        mui_canvas_clear(p_canvas);
        p_d->p_active_view->draw_cb(p_d->p_active_view, p_canvas);

        p_d->p_transition_new_buf = mui_mem_malloc((SCREEN_WIDTH * SCREEN_HEIGHT) / 8);
        if (p_d->p_transition_new_buf) {
            memcpy(p_d->p_transition_new_buf, fb, (SCREEN_WIDTH * SCREEN_HEIGHT) / 8);
        }
    }

    // fallback on alloc failure
    if (!p_d->p_transition_new_buf || !p_d->p_transition_old_buf) {
        mui_canvas_clear(p_canvas);
        p_d->p_active_view->draw_cb(p_d->p_active_view, p_canvas);
        return;
    }

    // Clear output buffer
    memset(fb, 0, (SCREEN_WIDTH * SCREEN_HEIGHT) / 8);

    switch (p_d->transition_dir) {
    case MUI_TRANSITION_DIR_RIGHT: {
        for (int page = 0; page < 8; page++) {
            int base = page * SCREEN_WIDTH;
            int remain = sw - t;
            if (remain > 0) {
                memcpy(fb + base, p_d->p_transition_old_buf + base + t, remain);
            }
            if (t > 0) {
                memcpy(fb + base + remain, p_d->p_transition_new_buf + base, t);
            }
        }
        break;
    }
    case MUI_TRANSITION_DIR_LEFT: {
        for (int page = 0; page < 8; page++) {
            int base = page * SCREEN_WIDTH;
            if (t > 0) {
                memcpy(fb + base, p_d->p_transition_new_buf + base, t);
            }
            memcpy(fb + base + t, p_d->p_transition_old_buf + base, sw - t);
        }
        break;
    }
    case MUI_TRANSITION_DIR_UP:
    case MUI_TRANSITION_DIR_DOWN: {
        for (int col = 0; col < SCREEN_WIDTH; col++) {
            uint8_t old_pixels[8];
            uint8_t new_pixels[8];
            for (int page = 0; page < 8; page++) {
                old_pixels[page] = p_d->p_transition_old_buf[page * SCREEN_WIDTH + col];
                new_pixels[page] = p_d->p_transition_new_buf[page * SCREEN_WIDTH + col];
            }

            uint64_t old_rows = 0, new_rows = 0;
            for (int page = 0; page < 8; page++) {
                old_rows |= ((uint64_t)old_pixels[page]) << (page * 8);
                new_rows |= ((uint64_t)new_pixels[page]) << (page * 8);
            }

            uint32_t shift = (t > 63) ? 63 : t;
            uint64_t out_rows;
            if (shift == 0) {
                out_rows = old_rows;
            } else if (p_d->transition_dir == MUI_TRANSITION_DIR_UP) {
                out_rows = (old_rows >> shift) | (new_rows << (64 - shift));
            } else {
                out_rows = (old_rows << shift) | (new_rows >> (64 - shift));
            }

            for (int page = 0; page < 8; page++) {
                fb[page * SCREEN_WIDTH + col] = (out_rows >> (page * 8)) & 0xFF;
            }
        }
        break;
    }
    }
}

static void mui_view_dispatcher_on_draw(mui_view_port_t *p_vp, mui_canvas_t *p_canvas) {
    mui_view_dispatcher_t *p_dispatcher = p_vp->user_data;

    if (p_dispatcher->transition_active) {
        if (p_dispatcher->p_transition_new_buf &&
            p_dispatcher->transition_anim.act_time >= p_dispatcher->transition_anim.time) {
            // Final frame: cleanup and draw normally
            p_dispatcher->transition_active = false;
            mui_mem_free(p_dispatcher->p_transition_old_buf);
            mui_mem_free(p_dispatcher->p_transition_new_buf);
            p_dispatcher->p_transition_old_buf = NULL;
            p_dispatcher->p_transition_new_buf = NULL;
            if (p_dispatcher->p_active_view) {
                mui_canvas_clear(p_canvas);
                p_dispatcher->p_active_view->draw_cb(p_dispatcher->p_active_view, p_canvas);
            }
            return;
        }
        mui_view_dispatcher_draw_transition(p_dispatcher, p_canvas);
        return;
    }

    if (p_dispatcher->p_active_view) {
        p_dispatcher->p_active_view->draw_cb(p_dispatcher->p_active_view, p_canvas);
    }
}

static void mui_view_dispatcher_set_curent_view(mui_view_dispatcher_t *p_dispatcher,
                                                mui_view_t *p_view) {
    mui_view_t *p_old_view = p_dispatcher->p_active_view;
    if (p_old_view == p_view) return;

    settings_data_t *p_settings = settings_get_data();
    bool can_animate = p_settings->anim_enabled && p_old_view != NULL && p_view != NULL;

    if (!can_animate) {
        if (p_old_view) mui_view_exit(p_old_view);
        p_dispatcher->p_active_view = p_view;
        if (p_view) {
            mui_view_enter(p_view);
            mui_view_port_enable_set(p_dispatcher->p_view_port, true);
            mui_view_port_update(p_dispatcher->p_view_port);
        } else {
            mui_view_port_enable_set(p_dispatcher->p_view_port, false);
        }
        return;
    }

    // Stop any in-flight transition
    if (p_dispatcher->transition_active) {
        mui_anim_stop(&p_dispatcher->transition_anim);
        if (p_dispatcher->p_transition_old_buf) {
            mui_mem_free(p_dispatcher->p_transition_old_buf);
            p_dispatcher->p_transition_old_buf = NULL;
        }
        if (p_dispatcher->p_transition_new_buf) {
            mui_mem_free(p_dispatcher->p_transition_new_buf);
            p_dispatcher->p_transition_new_buf = NULL;
        }
        p_dispatcher->transition_active = false;
    }

    mui_update_now(mui());
    p_dispatcher->p_transition_old_buf = mui_mem_malloc((SCREEN_WIDTH * SCREEN_HEIGHT) / 8);
    if (!p_dispatcher->p_transition_old_buf) {
        if (p_old_view) mui_view_exit(p_old_view);
        p_dispatcher->p_active_view = p_view;
        mui_view_enter(p_view);
        mui_view_port_enable_set(p_dispatcher->p_view_port, true);
        mui_view_port_update(p_dispatcher->p_view_port);
        return;
    }

    memcpy(p_dispatcher->p_transition_old_buf, u8g2_GetBufferPtr(&mui()->u8g2),
           (SCREEN_WIDTH * SCREEN_HEIGHT) / 8);

    if (p_old_view) mui_view_exit(p_old_view);
    p_dispatcher->p_active_view = p_view;
    p_dispatcher->p_transition_new_buf = NULL;
    mui_view_enter(p_view);
    mui_view_port_enable_set(p_dispatcher->p_view_port, true);
    p_dispatcher->transition_active = true;

    int32_t end_val = (p_dispatcher->transition_dir == MUI_TRANSITION_DIR_UP ||
                       p_dispatcher->transition_dir == MUI_TRANSITION_DIR_DOWN)
                      ? SCREEN_HEIGHT : SCREEN_WIDTH;

    uint32_t duration = p_settings->transition_duration_ms;
    if (duration == 0) duration = 200;

    mui_anim_init(&p_dispatcher->transition_anim);
    mui_anim_set_var(&p_dispatcher->transition_anim, p_dispatcher);
    mui_anim_set_exec_cb(&p_dispatcher->transition_anim, mui_view_dispatcher_transition_exec_cb);
    mui_anim_set_path_cb(&p_dispatcher->transition_anim, lv_anim_path_linear);
    mui_anim_set_values(&p_dispatcher->transition_anim, 0, end_val);
    mui_anim_set_time(&p_dispatcher->transition_anim, duration);
    mui_anim_start(&p_dispatcher->transition_anim);
}

static void mui_view_dispatcher_on_input(mui_view_port_t *p_vp,
                                         mui_input_event_t *p_event) {
    mui_view_dispatcher_t *p_dispatcher = p_vp->user_data;
    if (p_dispatcher->p_active_view) {
        p_dispatcher->p_active_view->input_cb(p_dispatcher->p_active_view, p_event);
    }
}

mui_view_dispatcher_t *mui_view_dispatcher_create() {
    mui_view_dispatcher_t *p_dsp = mui_mem_malloc(sizeof(mui_view_dispatcher_t));
    mui_view_dict_init(p_dsp->views);
    p_dsp->p_view_port = mui_view_port_create();
    p_dsp->p_view_port->draw_cb = mui_view_dispatcher_on_draw;
    p_dsp->p_view_port->input_cb = mui_view_dispatcher_on_input;
    p_dsp->p_view_port->user_data = p_dsp;
    p_dsp->p_active_view = NULL;
    p_dsp->transition_dir = MUI_TRANSITION_DIR_RIGHT;
    p_dsp->transition_active = false;
    p_dsp->p_transition_old_buf = NULL;
    p_dsp->p_transition_new_buf = NULL;
    return p_dsp;
}
void mui_view_dispatcher_free(mui_view_dispatcher_t *p_dispatcher) {
    if (p_dispatcher->transition_active) {
        mui_anim_stop(&p_dispatcher->transition_anim);
    }
    mui_mem_free(p_dispatcher->p_transition_old_buf);
    mui_mem_free(p_dispatcher->p_transition_new_buf);
    mui_view_dict_clear(p_dispatcher->views);
    mui_view_port_free(p_dispatcher->p_view_port);

    mui_mem_free(p_dispatcher);
}
void mui_view_dispatcher_add_view(mui_view_dispatcher_t *p_dispatcher, uint32_t view_id,
                                  mui_view_t *p_view) {
    mui_view_dict_set_at(p_dispatcher->views, view_id, p_view);
}
void mui_view_dispatcher_remove_view(mui_view_dispatcher_t *p_dispatcher,
                                     uint32_t view_id) {
    mui_view_t *p_view = *mui_view_dict_get(p_dispatcher->views, view_id);
    if (p_view == NULL) {
        return;
    }

    if (p_dispatcher->p_active_view == p_view) {
        mui_view_dispatcher_set_curent_view(p_dispatcher, NULL);
    }
}
void mui_view_dispatcher_attach(mui_view_dispatcher_t *p_dispatcher, mui_layer_t layer) {
    mui_add_view_port(mui(), p_dispatcher->p_view_port, layer);
}
void mui_view_dispatcher_detach(mui_view_dispatcher_t *p_dispatcher, mui_layer_t layer) {
    mui_view_port_enable_set(p_dispatcher->p_view_port, false);
    mui_remove_view_port(mui(), p_dispatcher->p_view_port, layer);
}
void mui_view_dispatcher_switch_to_view(mui_view_dispatcher_t *p_dispatcher,
                                        uint32_t view_id) {
    if (view_id == VIEW_NONE) {
        mui_view_dispatcher_set_curent_view(p_dispatcher, NULL);
    } else {
        mui_view_t *p_view = *mui_view_dict_get(p_dispatcher->views, view_id);
        if (p_view) {
            mui_view_dispatcher_set_curent_view(p_dispatcher, p_view);
        }
    }
}
void mui_view_dispatcher_set_transition_dir(mui_view_dispatcher_t *p_dispatcher, mui_transition_dir_t dir) {
    p_dispatcher->transition_dir = dir;
}
