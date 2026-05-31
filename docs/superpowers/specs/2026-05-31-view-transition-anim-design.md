# View Transition Animation Design

## Summary

Add configurable slide transitions to `mui_view_dispatcher` when switching views, driven by the existing `mui_anim` engine and gated by `settings.anim_enabled`.

## Architecture

### New Types

```c
typedef enum {
    MUI_TRANSITION_DIR_RIGHT,   // new view slides in from right, old out left (forward)
    MUI_TRANSITION_DIR_LEFT,    // new view slides in from left, old out right (back)
    MUI_TRANSITION_DIR_UP,      // new view slides up from bottom, old out top
    MUI_TRANSITION_DIR_DOWN,    // new view slides down from top, old out bottom
} mui_transition_dir_t;
```

### Struct Changes (`mui_view_dispatcher_t`)

```c
typedef struct {
    mui_view_port_t* p_view_port;
    mui_view_dict_t views;
    mui_view_t* p_active_view;
    // --- transition fields ---
    mui_transition_dir_t transition_dir;  // direction, configurable by caller
    mui_anim_t transition_anim;           // anim instance
    uint8_t *p_transition_old_buf;        // malloc'd 1024-byte old frame snapshot
    uint8_t *p_transition_new_buf;        // malloc'd 1024-byte new frame snapshot
    bool transition_active;               // draw-cb uses composited output
} mui_view_dispatcher_t;
```

### Public API Additions

```c
void mui_view_dispatcher_set_transition_dir(mui_view_dispatcher_t *p_d, mui_transition_dir_t dir);
void mui_view_dispatcher_switch_to_view(mui_view_dispatcher_t *p_d, uint32_t view_id); // modified
```

## Transition Flow

```
switch_to_view(view_id)
  │
  ├─ !settings.anim_enabled or no old view or view not found
  │   → instant switch (unchanged path)
  │
   └─ transition path:
      1. mui_update_now() → force full redraw of current frame
      2. snapshot u8g2 buffer → p_transition_old_buf (1024 bytes)
      3. exit_cb(old_view)
      4. enter_cb(new_view)
      5. set transition_active = true
      6. start mui_anim: 0 → SCREEN_WIDTH/HEIGHT, path=linear, time=settings
      7. trigger redraw → first on_draw with transition_active:
         - draws new view (first capture), creates new_buf
         - at anim offset=0 → only old visible
      8. each anim tick (50Hz): on_draw composites old_buf + new_buf at offset
      9. anim complete → transition_active=false, free bufs, normal redraw
```

### Composited Draw (per animation tick)

The `mui_view_dispatcher_on_draw` callback checks `transition_active`. When true, it composites old and new frame buffers with the current offset instead of calling the active view's `draw_cb`.

**Horizontal (LEFT/RIGHT):** Direct byte-column memcpy with offset:
- `RIGHT:` `output[0..127-t] = old[t..127]`, `output[128-t..127] = new[0..t-1]`
- `LEFT:`  `output[0..t-1] = new[0..t-1]`, `output[t..127] = old[0..127-t]`

**Vertical (UP/DOWN):** Row-by-row bit manipulation across page boundaries (8 pages × 128 columns):
- Each column's 8 bytes (8 pages × 1 byte/column) represent 64 pixel rows
- Slide shifts pixel rows across adjacent pages

### Animation Config

- **Duration:** defaults to 200ms, overridable via new `settings.transition_duration_ms` field
- **Easing:** `lv_anim_path_linear` (no fade)
- **Cancellation:** if a new `switch_to_view` arrives mid-transition, stop current anim and restart with the new target

## Integration Points

### Scene Dispatcher

`mui_scene_dispatcher_next_scene()` sets `MUI_TRANSITION_DIR_RIGHT` before calling view switch.
`mui_scene_dispatcher_previous_scene()` sets `MUI_TRANSITION_DIR_LEFT` before calling view switch.

### Settings

Respects existing `settings.anim_enabled` (default: false). No new settings toggle added; reuses the existing one.

### Memory

- `p_transition_old_buf` + `p_transition_new_buf`: 2048 bytes total, malloc'd on transition start, freed on completion
- Total peak: ~2KB extra during transitions (within 18KB heap budget)

## Files Modified

| File | Change |
|------|--------|
| `mui/mui_view_dispatcher.h` | New enum, struct fields, API |
| `mui/mui_view_dispatcher.c` | Transition logic in set_curent_view, on_draw, compositing |
| `mui/mui_scene_dispatcher.c` | Set transition_dir before view switch |
| `mod/settings.h` | (optional) `transition_duration_ms` field |

## Error Handling

- malloc failure for old_buf → fall back to instant switch
- mid-transition switch → stop anim, free buf, start new transition
- view removed during transition → stop anim, treat as instant switch
