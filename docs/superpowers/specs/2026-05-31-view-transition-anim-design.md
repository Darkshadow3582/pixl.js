# View 切换动画设计

## 概述

在 `mui_view_dispatcher` 切换 view 时添加可配置的滑动过渡动画，复用已有的 `mui_anim` 引擎，受 `settings.anim_enabled` 控制。

## 架构

### 新增类型

```c
typedef enum {
    MUI_TRANSITION_DIR_RIGHT,   // 新 view 从右侧滑入，旧 view 从左侧滑出（前进）
    MUI_TRANSITION_DIR_LEFT,    // 新 view 从左侧滑入，旧 view 从右侧滑出（后退）
    MUI_TRANSITION_DIR_UP,      // 新 view 从下方滑入，旧 view 从上方滑出
    MUI_TRANSITION_DIR_DOWN,    // 新 view 从上方滑入，旧 view 从下方滑出
} mui_transition_dir_t;
```

### 结构体变更（`mui_view_dispatcher_t`）

```c
typedef struct {
    mui_view_port_t* p_view_port;
    mui_view_dict_t views;
    mui_view_t* p_active_view;
    // --- 过渡动画字段 ---
    mui_transition_dir_t transition_dir;  // 滑动方向，由调用方配置
    mui_anim_t transition_anim;           // 动画实例
    uint8_t *p_transition_old_buf;        // malloc 1024 字节旧帧快照
    uint8_t *p_transition_new_buf;        // malloc 1024 字节新帧快照
    bool transition_active;               // draw-cb 使用合成输出
} mui_view_dispatcher_t;
```

### 新增公开 API

```c
void mui_view_dispatcher_set_transition_dir(mui_view_dispatcher_t *p_d, mui_transition_dir_t dir);
void mui_view_dispatcher_switch_to_view(mui_view_dispatcher_t *p_d, uint32_t view_id); // 已修改
```

## 过渡流程

```
switch_to_view(view_id)
  │
  ├─ !settings.anim_enabled 或没有旧 view 或 view 未找到
  │   → 瞬间切换（原路径不变）
  │
   └─ 过渡动画路径：
      1. mui_update_now() → 强制全帧重绘当前画面
      2. 截图 u8g2 buffer → p_transition_old_buf（1024 字节）
      3. exit_cb(old_view)
      4. enter_cb(new_view)
      5. 设置 transition_active = true
      6. 启动 mui_anim：0 → SCREEN_WIDTH/HEIGHT，path=linear，时长=settings
      7. 触发重绘 → 首次调用 transition_active 下的 on_draw：
         - 绘制新 view（首次捕获），创建 new_buf
         - 动画偏移量=0 → 仅旧画面可见
      8. 每次动画 tick（50Hz）：on_draw 在偏移量处合成 old_buf + new_buf
      9. 动画完成 → transition_active=false，释放两个 buf，正常重绘
```

### 合成绘制（每次动画 tick）

`mui_view_dispatcher_on_draw` 回调检查 `transition_active`。为 true 时，它将新旧帧缓冲区按当前偏移量合成，而不是调用活跃 view 的 `draw_cb`。

**水平方向（LEFT/RIGHT）：** 直接按字节列 memcpy 带偏移：
- `RIGHT：` `output[0..127-t] = old[t..127]`，`output[128-t..127] = new[0..t-1]`
- `LEFT：`  `output[0..t-1] = new[0..t-1]`，`output[t..127] = old[0..127-t]`

**垂直方向（UP/DOWN）：** 逐行跨页面边界位操作（8 页 × 128 列）：
- 每列 8 字节（8 页 × 1 字节/列）表示 64 像素行
- 滑动将像素行跨相邻页移位

### 动画配置

- **时长：** 默认 200ms，可通过新字段 `settings.transition_duration_ms` 覆盖
- **缓动：** `lv_anim_path_linear`（无淡出）
- **取消：** 过渡过程中新的 `switch_to_view` 到达时，停止当前动画并重新以新目标启动

## 集成点

### Scene Dispatcher

`mui_scene_dispatcher_next_scene()` 在调用 view 切换前设置 `MUI_TRANSITION_DIR_RIGHT`。
`mui_scene_dispatcher_previous_scene()` 在调用 view 切换前设置 `MUI_TRANSITION_DIR_LEFT`。

### Settings

复用现有 `settings.anim_enabled`（默认 false）。不新增设置开关。

### 内存

- `p_transition_old_buf` + `p_transition_new_buf`：共 2048 字节，过渡开始时 malloc，完成时释放
- 峰值：过渡期间额外 ~2KB（在 18KB heap 预算内）

## 修改的文件

| 文件 | 变更 |
|------|------|
| `mui/mui_view_dispatcher.h` | 新枚举、结构体字段、API |
| `mui/mui_view_dispatcher.c` | set_curent_view、on_draw、合成中的过渡逻辑 |
| `mui/mui_scene_dispatcher.c` | 在 view 切换前设置 transition_dir |
| `mod/settings.h` |（可选）`transition_duration_ms` 字段 |

## 错误处理

- malloc 分配 old_buf 失败 → 回退到瞬间切换
- 过渡中切换 → 停止动画、释放 buf、启动新过渡
- 过渡中 view 被移除 → 停止动画，按瞬间切换处理
