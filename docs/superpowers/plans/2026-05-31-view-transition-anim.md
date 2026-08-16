# View 切换动画 实现计划

> **面向 AI 代理的工作者：** 必需子技能：使用 superpowers:subagent-driven-development（推荐）或 superpowers:executing-plans 逐任务实现此计划。步骤使用复选框（`- [ ]`）语法来跟踪进度。

**目标：** 在 `mui_view_dispatcher` 的 view 切换中加入可配置的滑动过渡动画。

**架构：** 切换 view 时截图当前帧到 buffer，enter 新 view 后截取新帧，用现有 mui_anim 引擎驱动偏移合成，在 on_draw 回调中输出合成帧。

**技术栈：** nRF52 SDK, u8g2, mui_anim, mui_view_dispatcher

---

### 任务 1：新增枚举和结构体字段

**文件：**
- 修改：`fw/application/src/mui/mui_view_dispatcher.h`

- [ ] **步骤 1：添加 transition_dir 枚举**

在 `#include` 之后、结构体定义之前添加：

```c
typedef enum {
    MUI_TRANSITION_DIR_RIGHT,   // 新 view 从右侧滑入，旧 view 从左侧滑出（前进）
    MUI_TRANSITION_DIR_LEFT,    // 新 view 从左侧滑入，旧 view 从右侧滑出（后退）
    MUI_TRANSITION_DIR_UP,      // 新 view 从下方滑入，旧 view 从上方滑出
    MUI_TRANSITION_DIR_DOWN,    // 新 view 从上方滑入，旧 view 从下方滑出
} mui_transition_dir_t;
```

- [ ] **步骤 2：扩展 mui_view_dispatcher_t 结构体**

```c
typedef struct {
    mui_view_port_t* p_view_port;
    mui_view_dict_t views;
    mui_view_t* p_active_view;
    // --- 过渡动画字段 ---
    mui_transition_dir_t transition_dir;
    mui_anim_t transition_anim;
    uint8_t *p_transition_old_buf;
    uint8_t *p_transition_new_buf;
    bool transition_active;
} mui_view_dispatcher_t;
```

- [ ] **步骤 3：添加 set_transition_dir 函数声明**

```c
void mui_view_dispatcher_set_transition_dir(mui_view_dispatcher_t *p_dispatcher, mui_transition_dir_t dir);
```

- [ ] **步骤 4：Commit**

```bash
git add fw/application/src/mui/mui_view_dispatcher.h
git commit -m "feat: add transition enum and struct fields to view dispatcher"
```

---

### 任务 2：实现 view dispatcher 过渡逻辑

**文件：**
- 修改：`fw/application/src/mui/mui_view_dispatcher.c`

- [ ] **步骤 1：在文件顶部添加新 include**

```c
#include "mui_anim.h"
#include "settings.h"
```

- [ ] **步骤 2：添加 set_transition_dir 实现**

```c
void mui_view_dispatcher_set_transition_dir(mui_view_dispatcher_t *p_dispatcher, mui_transition_dir_t dir) {
    p_dispatcher->transition_dir = dir;
}
```

- [ ] **步骤 3：添加合成绘制函数**

在 `mui_view_dispatcher_on_draw` 之前添加：

```c
static void mui_view_dispatcher_draw_transition(mui_view_dispatcher_t *p_d, mui_canvas_t *p_canvas) {
    u8g2_t *u8g2 = p_canvas->fb;
    uint8_t *fb = u8g2_GetBufferPtr(u8g2);
    int32_t t = p_d->transition_anim.current_value;
    int32_t sw = (int32_t)mui_canvas_get_width(p_canvas);
    int32_t sh = (int32_t)mui_canvas_get_height(p_canvas);

    // First time: let new view draw and capture its buffer
    if (!p_d->p_transition_new_buf) {
        mui_canvas_clear(p_canvas);
        p_d->p_active_view->draw_cb(p_d->p_active_view, p_canvas);

        p_d->p_transition_new_buf = mui_mem_malloc(1024);
        if (p_d->p_transition_new_buf) {
            memcpy(p_d->p_transition_new_buf, fb, 1024);
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
    case MUI_TRANSITION_DIR_DOWN:
        // Vertical: for each column, shift pixel rows across page bytes
        for (int col = 0; col < SCREEN_WIDTH; col++) {
            uint8_t old_pixels[SCREEN_HEIGHT / 8];
            uint8_t new_pixels[SCREEN_HEIGHT / 8];
            for (int page = 0; page < 8; page++) {
                old_pixels[page] = p_d->p_transition_old_buf[page * SCREEN_WIDTH + col];
                new_pixels[page] = p_d->p_transition_new_buf[page * SCREEN_WIDTH + col];
            }

            uint64_t old_rows = 0, new_rows = 0;
            for (int page = 0; page < 8; page++) {
                old_rows |= ((uint64_t)old_pixels[page]) << (page * 8);
                new_rows |= ((uint64_t)new_pixels[page]) << (page * 8);
            }

            int dir = (p_d->transition_dir == MUI_TRANSITION_DIR_UP) ? 1 : -1;
            int shift = (dir * t) & 63;
            uint64_t out_rows = 0;
            if (shift >= 0) {
                out_rows = (old_rows >> shift) | (new_rows << (64 - shift));
            } else {
                shift = -shift;
                out_rows = (old_rows << shift) | (new_rows >> (64 - shift));
            }

            for (int page = 0; page < 8; page++) {
                fb[page * SCREEN_WIDTH + col] = (out_rows >> (page * 8)) & 0xFF;
            }
        }
        break;
    }
}
```

- [ ] **步骤 4：修改 on_draw 回调**

添加 `transition_active` 检查：

```c
static void mui_view_dispatcher_on_draw(mui_view_port_t *p_vp, mui_canvas_t *p_canvas) {
    mui_view_dispatcher_t *p_dispatcher = p_vp->user_data;

    if (p_dispatcher->transition_active) {
        // Check if anim completed
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
```

- [ ] **步骤 5：添加动画执行回调**

```c
static void mui_view_dispatcher_transition_exec_cb(void *var, int32_t value) {
    (void)value;
    mui_view_dispatcher_t *p_d = (mui_view_dispatcher_t *)var;
    mui_view_port_update(p_d->p_view_port);
}
```

- [ ] **步骤 6：修改 set_curent_view 添加过渡路径**

```c
static void mui_view_dispatcher_set_curent_view(mui_view_dispatcher_t *p_dispatcher,
                                                mui_view_t *p_view) {
    mui_view_t *p_old_view = p_dispatcher->p_active_view;
    if (p_old_view == p_view) return;

    settings_data_t *p_settings = settings_get_data();
    bool can_animate = p_settings->anim_enabled && p_old_view != NULL && p_view != NULL;

    if (!can_animate) {
        // Instant path
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

    // Transition path
    mui_update_now(mui());
    p_dispatcher->p_transition_old_buf = mui_mem_malloc((SCREEN_WIDTH * SCREEN_HEIGHT) / 8);
    if (!p_dispatcher->p_transition_old_buf) {
        // malloc fail, fallback to instant
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
```

- [ ] **步骤 7：在 create 中初始化新字段**

```c
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
```

- [ ] **步骤 8：Commit**

```bash
git add fw/application/src/mui/mui_view_dispatcher.c
git commit -m "feat: implement view transition animation with slide compositing"
```

---

### 任务 3：集成 Scene Dispatcher

**文件：**
- 修改：`fw/application/src/mui/mui_scene_dispatcher.h`
- 修改：`fw/application/src/mui/mui_scene_dispatcher.c`

- [ ] **步骤 1：在头文件中添加 set_view_dispatcher API**

在 `mui_scene_dispatcher.h` 中的结构体定义之后添加：

```c
// forward declaration
typedef struct mui_view_dispatcher_s mui_view_dispatcher_t;

void mui_scene_dispatcher_set_view_dispatcher(mui_scene_dispatcher_t *p_dispatcher,
                                              mui_view_dispatcher_t *p_view_dispatcher);
```

- [ ] **步骤 2：在结构体中添加 view_dispatcher 字段**

```c
typedef struct {
    void *user_data;
    const mui_scene_t *p_scene_defines;
    uint32_t scene_num;
    uint32_t default_scene_id;
    scene_id_stack_t scene_id_stack;
    mui_view_dispatcher_t *p_view_dispatcher;  // <-- 新增
} mui_scene_dispatcher_t;
```

- [ ] **步骤 3：在 scene_dispatcher.c 中添加 include 和 setter**

```c
#include "mui_view_dispatcher.h"

void mui_scene_dispatcher_set_view_dispatcher(mui_scene_dispatcher_t *p_dispatcher,
                                              mui_view_dispatcher_t *p_view_dispatcher) {
    p_dispatcher->p_view_dispatcher = p_view_dispatcher;
}
```

- [ ] **步骤 4：在 next_scene 和 previous_scene 中设置方向**

完整修改 `mui_scene_dispatcher_next_scene`：

```c
void mui_scene_dispatcher_next_scene(mui_scene_dispatcher_t *p_dispatcher, uint32_t scene_id) {
    if (p_dispatcher->p_view_dispatcher) {
        mui_view_dispatcher_set_transition_dir(p_dispatcher->p_view_dispatcher, MUI_TRANSITION_DIR_RIGHT);
    }
    if (scene_id_stack_size(p_dispatcher->scene_id_stack) > 0) {
        uint32_t cur_scene_id = *scene_id_stack_back(p_dispatcher->scene_id_stack);
        p_dispatcher->p_scene_defines[cur_scene_id].exit_cb(p_dispatcher->user_data);
    }
    scene_id_stack_push_back(p_dispatcher->scene_id_stack, scene_id);
    p_dispatcher->p_scene_defines[scene_id].enter_cb(p_dispatcher->user_data);
}
```

完整修改 `mui_scene_dispatcher_previous_scene`：

```c
void mui_scene_dispatcher_previous_scene(mui_scene_dispatcher_t *p_dispatcher) {
    if (p_dispatcher->p_view_dispatcher) {
        mui_view_dispatcher_set_transition_dir(p_dispatcher->p_view_dispatcher, MUI_TRANSITION_DIR_LEFT);
    }
    if (scene_id_stack_size(p_dispatcher->scene_id_stack) > 0) {
        uint32_t cur_scene_id;
        scene_id_stack_pop_back(&cur_scene_id, p_dispatcher->scene_id_stack);
        if (scene_id_stack_size(p_dispatcher->scene_id_stack) == 0) {
            p_dispatcher->p_scene_defines[cur_scene_id].exit_cb(p_dispatcher->user_data);
            p_dispatcher->p_scene_defines[p_dispatcher->default_scene_id].enter_cb(p_dispatcher->user_data);
        } else {
            uint32_t prev_scene_id = *scene_id_stack_back(p_dispatcher->scene_id_stack);
            p_dispatcher->p_scene_defines[cur_scene_id].exit_cb(p_dispatcher->user_data);
            p_dispatcher->p_scene_defines[prev_scene_id].enter_cb(p_dispatcher->user_data);
        }
    }
}
```

完整修改 `mui_scene_dispatcher_back_scene`：

```c
void mui_scene_dispatcher_back_scene(mui_scene_dispatcher_t *p_dispatcher, uint32_t step) {
    if (p_dispatcher->p_view_dispatcher) {
        mui_view_dispatcher_set_transition_dir(p_dispatcher->p_view_dispatcher, MUI_TRANSITION_DIR_LEFT);
    }
    uint32_t cur_scene_id = *scene_id_stack_back(p_dispatcher->scene_id_stack);
    uint32_t pre_scene_id;
    while (scene_id_stack_size(p_dispatcher->scene_id_stack) > 0 && step > 0) {
        scene_id_stack_pop_back(&pre_scene_id, p_dispatcher->scene_id_stack);
        step--;
    }
    if (scene_id_stack_size(p_dispatcher->scene_id_stack) == 0) {
        p_dispatcher->p_scene_defines[cur_scene_id].exit_cb(p_dispatcher->user_data);
        p_dispatcher->p_scene_defines[p_dispatcher->default_scene_id].enter_cb(p_dispatcher->user_data);
    } else {
        uint32_t prev_scene_id = *scene_id_stack_back(p_dispatcher->scene_id_stack);
        p_dispatcher->p_scene_defines[cur_scene_id].exit_cb(p_dispatcher->user_data);
        p_dispatcher->p_scene_defines[prev_scene_id].enter_cb(p_dispatcher->user_data);
    }
}
```

- [ ] **步骤 5：在 create 中初始化新字段为 NULL**

```c
p_dispatcher->p_view_dispatcher = NULL;
```

- [ ] **步骤 6：Commit**

```bash
git add fw/application/src/mui/mui_scene_dispatcher.h fw/application/src/mui/mui_scene_dispatcher.c
git commit -m "feat: integrate scene dispatcher with transition direction"
```

---

### 任务 4：添加 transition_duration_ms 到 settings

**文件：**
- 修改：`fw/application/src/mod/settings.h`

- [ ] **步骤 1：在结构体中添加字段**

在 `settings_data_t` 中的适当位置添加（例如放在 `anim_enabled` 附近或末尾）：

```c
uint16_t transition_duration_ms;  // view 切换动画时长 (ms)，0 表示使用默认值 200ms
```

- [ ] **步骤 2：在默认值结构中初始化**

在 `def_settings_data` 中添加（例如在 `anim_enabled` 附近）：

```c
.transition_duration_ms = 200,
```

- [ ] **步骤 3：Commit**

```bash
git add fw/application/src/mod/settings.h
git commit -m "feat: add transition_duration_ms to settings"
```

---

### 任务 5：构建验证

**文件：**
- 无需修改文件

- [ ] **步骤 1：执行编译**

```bash
make app
```

运行：`make app`
预期：编译成功，无错误

- [ ] **步骤 2：如果编译失败，修复错误并重新编译**

- [ ] **步骤 3：Commit 最终版本**

```bash
git add -A
git commit -m "chore: fix build issues"
```

---

### 任务 6：推送远程

- [ ] **步骤 1：推送 feature 分支**

```bash
git push origin feature/view-transition-anim
```
