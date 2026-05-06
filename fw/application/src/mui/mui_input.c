#include "mui_input.h"

#include "app_timer.h"
#include "mui_core.h"
#include "mui_event.h"
#include "bsp_btn.h"
#include "cache.h"
#include "nrf_error.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "settings.h"

#define RETURN_KEY_POLL_INTERVAL_MS 20
#define RETURN_KEY_DEBOUNCE_TICKS 2
#define RETURN_KEY_DETECT_TIMEOUT_MS 8000
#define RETURN_KEY_CANDIDATES_MAX 32

APP_TIMER_DEF(m_return_key_poll_timer_id);
APP_TIMER_DEF(m_return_key_detect_timeout_timer_id);

typedef struct {
    bool timers_initialized;
    bool poll_timer_started;
    bool detection_active;
    mui_input_return_key_detect_cb_t detect_cb;
    void *detect_user_data;
    uint8_t detect_pins[RETURN_KEY_CANDIDATES_MAX];
    uint8_t detect_stable_pressed[RETURN_KEY_CANDIDATES_MAX];
    uint8_t detect_debounce_cnt[RETURN_KEY_CANDIDATES_MAX];
    uint8_t detect_pin_count;
    uint8_t runtime_pin;
    uint8_t runtime_stable_pressed;
    uint8_t runtime_debounce_cnt;
    bool runtime_was_pressed;
} mui_input_return_key_ctx_t;

static mui_input_return_key_ctx_t m_return_key_ctx = {.runtime_pin = SETTINGS_RETURN_KEY_PIN_UNCONFIGURED};

static void mui_input_post_event(mui_input_event_t *p_input_event) {
    uint32_t arg = p_input_event->type;
    arg <<= 8;
    arg += p_input_event->key;
    mui_event_t mui_event = {.id = MUI_EVENT_ID_INPUT, .arg_int = arg};
    mui_post(mui(), &mui_event);
}

static bool mui_input_return_key_is_reserved_pin(uint8_t pin) {
#ifdef BUTTON_1
    if (pin == BUTTON_1) {
        return true;
    }
#endif
#ifdef BUTTON_2
    if (pin == BUTTON_2) {
        return true;
    }
#endif
#ifdef BUTTON_3
    if (pin == BUTTON_3) {
        return true;
    }
#endif
#ifdef LED_1
    if (pin == LED_1) {
        return true;
    }
#endif
#ifdef LCD_CS_PIN
    if (pin == LCD_CS_PIN) {
        return true;
    }
#endif
#ifdef LCD_RESET_PIN
    if (pin == LCD_RESET_PIN) {
        return true;
    }
#endif
#ifdef LCD_BL_PIN
    if (pin == LCD_BL_PIN) {
        return true;
    }
#endif
#ifdef LCD_DC_PIN
    if (pin == LCD_DC_PIN) {
        return true;
    }
#endif
#ifdef NRFX_SPIM_SCK_PIN
    if (pin == NRFX_SPIM_SCK_PIN) {
        return true;
    }
#endif
#ifdef NRFX_SPIM_MOSI_PIN
    if (pin == NRFX_SPIM_MOSI_PIN) {
        return true;
    }
#endif
#ifdef NRFX_SPIM_MISO_PIN
    if (pin == NRFX_SPIM_MISO_PIN) {
        return true;
    }
#endif
#ifdef FLASH_CS_PIN
    if (pin == FLASH_CS_PIN) {
        return true;
    }
#endif
#ifdef CHRG_PIN
    if (pin == CHRG_PIN) {
        return true;
    }
#endif
#ifdef ADC_PIN
    if (pin == ADC_PIN) {
        return true;
    }
#endif
    return false;
}

static void mui_input_return_key_finish_detection(bool detected, uint8_t pin) {
    if (!m_return_key_ctx.detection_active) {
        return;
    }

    app_timer_stop(m_return_key_detect_timeout_timer_id);
    m_return_key_ctx.detection_active = false;

    mui_input_return_key_detect_cb_t cb = m_return_key_ctx.detect_cb;
    void *user_data = m_return_key_ctx.detect_user_data;
    m_return_key_ctx.detect_cb = NULL;
    m_return_key_ctx.detect_user_data = NULL;
    m_return_key_ctx.detect_pin_count = 0;

    if (cb) {
        cb(detected, pin, user_data);
    }
}

static bool mui_input_return_key_read_pressed(uint8_t pin) { return nrf_gpio_pin_read(pin) == 0; }

static void mui_input_return_key_prepare_detection_pins() {
    m_return_key_ctx.detect_pin_count = 0;

    for (uint8_t pin = 0; pin <= SETTINGS_RETURN_KEY_MAX_PIN && m_return_key_ctx.detect_pin_count < RETURN_KEY_CANDIDATES_MAX;
         pin++) {
        if (mui_input_return_key_is_reserved_pin(pin)) {
            continue;
        }

        uint8_t index = m_return_key_ctx.detect_pin_count;
        m_return_key_ctx.detect_pins[index] = pin;
        nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLUP);
        m_return_key_ctx.detect_stable_pressed[index] = mui_input_return_key_read_pressed(pin) ? 1 : 0;
        m_return_key_ctx.detect_debounce_cnt[index] = 0;
        m_return_key_ctx.detect_pin_count++;
    }
}

static void mui_input_return_key_reset_runtime_state() {
    m_return_key_ctx.runtime_pin = SETTINGS_RETURN_KEY_PIN_UNCONFIGURED;
    m_return_key_ctx.runtime_stable_pressed = 0;
    m_return_key_ctx.runtime_debounce_cnt = 0;
    m_return_key_ctx.runtime_was_pressed = false;
}

static bool mui_input_return_key_get_active_pin(uint8_t *pin_out) {
    const settings_data_t *p_settings = settings_get_data();
    if (!p_settings->return_key_enabled) {
        return false;
    }
    if (p_settings->return_key_pin == SETTINGS_RETURN_KEY_PIN_UNCONFIGURED ||
        p_settings->return_key_pin > SETTINGS_RETURN_KEY_MAX_PIN) {
        return false;
    }
    *pin_out = p_settings->return_key_pin;
    return true;
}

static void mui_input_return_key_poll_detection() {
    if (!m_return_key_ctx.detection_active) {
        return;
    }

    for (uint8_t i = 0; i < m_return_key_ctx.detect_pin_count; i++) {
        bool pressed = mui_input_return_key_read_pressed(m_return_key_ctx.detect_pins[i]);
        bool stable_pressed = m_return_key_ctx.detect_stable_pressed[i] != 0;

        if (pressed == stable_pressed) {
            m_return_key_ctx.detect_debounce_cnt[i] = 0;
            continue;
        }

        if (m_return_key_ctx.detect_debounce_cnt[i] < UINT8_MAX) {
            m_return_key_ctx.detect_debounce_cnt[i]++;
        }
        if (m_return_key_ctx.detect_debounce_cnt[i] < RETURN_KEY_DEBOUNCE_TICKS) {
            continue;
        }

        m_return_key_ctx.detect_debounce_cnt[i] = 0;
        m_return_key_ctx.detect_stable_pressed[i] = pressed ? 1 : 0;
        if (pressed) {
            mui_input_return_key_finish_detection(true, m_return_key_ctx.detect_pins[i]);
            return;
        }
    }
}

static void mui_input_return_key_poll_runtime() {
    if (m_return_key_ctx.detection_active) {
        return;
    }

    uint8_t return_key_pin = SETTINGS_RETURN_KEY_PIN_UNCONFIGURED;
    if (!mui_input_return_key_get_active_pin(&return_key_pin)) {
        mui_input_return_key_reset_runtime_state();
        return;
    }

    if (return_key_pin != m_return_key_ctx.runtime_pin) {
        m_return_key_ctx.runtime_pin = return_key_pin;
        nrf_gpio_cfg_input(return_key_pin, NRF_GPIO_PIN_PULLUP);
        m_return_key_ctx.runtime_stable_pressed = mui_input_return_key_read_pressed(return_key_pin) ? 1 : 0;
        m_return_key_ctx.runtime_debounce_cnt = 0;
        m_return_key_ctx.runtime_was_pressed = m_return_key_ctx.runtime_stable_pressed != 0;
        return;
    }

    bool pressed = mui_input_return_key_read_pressed(return_key_pin);
    bool stable_pressed = m_return_key_ctx.runtime_stable_pressed != 0;
    if (pressed == stable_pressed) {
        m_return_key_ctx.runtime_debounce_cnt = 0;
        return;
    }

    if (m_return_key_ctx.runtime_debounce_cnt < UINT8_MAX) {
        m_return_key_ctx.runtime_debounce_cnt++;
    }
    if (m_return_key_ctx.runtime_debounce_cnt < RETURN_KEY_DEBOUNCE_TICKS) {
        return;
    }

    m_return_key_ctx.runtime_debounce_cnt = 0;
    m_return_key_ctx.runtime_stable_pressed = pressed ? 1 : 0;

    mui_input_event_t event = {.key = INPUT_KEY_BACK, .type = INPUT_TYPE_PRESS};
    if (pressed) {
        mui_input_post_event(&event);
        m_return_key_ctx.runtime_was_pressed = true;
    } else {
        event.type = INPUT_TYPE_RELEASE;
        mui_input_post_event(&event);

        if (m_return_key_ctx.runtime_was_pressed) {
            event.type = INPUT_TYPE_SHORT;
            mui_input_post_event(&event);
        }
        m_return_key_ctx.runtime_was_pressed = false;
    }
}

static void mui_input_return_key_poll_timer_handler(void *p_context) {
    (void)p_context;
    mui_input_return_key_poll_detection();
    mui_input_return_key_poll_runtime();
}

static void mui_input_return_key_detect_timeout_timer_handler(void *p_context) {
    (void)p_context;
    mui_input_return_key_finish_detection(false, SETTINGS_RETURN_KEY_PIN_UNCONFIGURED);
}

static void mui_input_return_key_init_timers() {
    if (m_return_key_ctx.timers_initialized) {
        return;
    }

    ret_code_t err_code = app_timer_create(&m_return_key_poll_timer_id, APP_TIMER_MODE_REPEATED,
                                           mui_input_return_key_poll_timer_handler);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_create(&m_return_key_detect_timeout_timer_id, APP_TIMER_MODE_SINGLE_SHOT,
                                mui_input_return_key_detect_timeout_timer_handler);
    APP_ERROR_CHECK(err_code);
    m_return_key_ctx.timers_initialized = true;
}

static void mui_input_return_key_start_poll_timer() {
    if (m_return_key_ctx.poll_timer_started) {
        return;
    }

    ret_code_t err_code =
        app_timer_start(m_return_key_poll_timer_id, APP_TIMER_TICKS(RETURN_KEY_POLL_INTERVAL_MS), NULL);
    APP_ERROR_CHECK(err_code);
    m_return_key_ctx.poll_timer_started = true;
}

void mui_input_on_bsp_btn_event(uint8_t btn, bsp_btn_event_t evt) {
    input_key_t mapped_key = (input_key_t)btn;

    switch (evt) {

    case BSP_BTN_EVENT_PRESSED: {
        NRF_LOG_DEBUG("Key %d pressed", btn);
        mui_input_event_t input_event = {.key = mapped_key, .type = INPUT_TYPE_PRESS};
        mui_input_post_event(&input_event);
        break;
    }

    case BSP_BTN_EVENT_RELEASED: {
        NRF_LOG_DEBUG("Key %d released", btn);
        mui_input_event_t input_event = {.key = mapped_key, .type = INPUT_TYPE_RELEASE};
        mui_input_post_event(&input_event);
        break;
    }

    case BSP_BTN_EVENT_SHORT: {
        NRF_LOG_DEBUG("Key %d short push", btn);
        mui_input_event_t input_event = {.key = mapped_key,
                                         .type = INPUT_TYPE_SHORT};
        mui_input_post_event(&input_event);
        break;
    }

    case BSP_BTN_EVENT_LONG: {
        NRF_LOG_DEBUG("Key %d long push", btn);
        mui_input_event_t input_event = {.key = mapped_key,
                                         .type = INPUT_TYPE_LONG};
        mui_input_post_event(&input_event);

        break;
    }

    case BSP_BTN_EVENT_REPEAT: {
        NRF_LOG_DEBUG("Key %d repeat push", btn);
        mui_input_event_t input_event = {.key = mapped_key,
                                         .type = INPUT_TYPE_REPEAT};
        mui_input_post_event(&input_event);

        break;
    }

    default:
        break;
    }
}


void mui_input_init() {
    bsp_btn_init(mui_input_on_bsp_btn_event);
    mui_input_return_key_init_timers();
    mui_input_return_key_start_poll_timer();
}

bool mui_input_return_key_detect_start(mui_input_return_key_detect_cb_t cb, void *user_data) {
    if (m_return_key_ctx.detection_active) {
        return false;
    }

    mui_input_return_key_init_timers();
    mui_input_return_key_start_poll_timer();

    m_return_key_ctx.detect_cb = cb;
    m_return_key_ctx.detect_user_data = user_data;
    m_return_key_ctx.detection_active = true;
    mui_input_return_key_prepare_detection_pins();

    if (m_return_key_ctx.detect_pin_count == 0) {
        mui_input_return_key_finish_detection(false, SETTINGS_RETURN_KEY_PIN_UNCONFIGURED);
        return false;
    }

    ret_code_t err_code = app_timer_start(m_return_key_detect_timeout_timer_id,
                                          APP_TIMER_TICKS(RETURN_KEY_DETECT_TIMEOUT_MS), NULL);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_WARNING("Failed to start return-key detect timeout timer: %d", err_code);
        mui_input_return_key_finish_detection(false, SETTINGS_RETURN_KEY_PIN_UNCONFIGURED);
        return false;
    }

    return true;
}

void mui_input_return_key_detect_cancel() {
    if (!m_return_key_ctx.detection_active) {
        return;
    }

    app_timer_stop(m_return_key_detect_timeout_timer_id);
    m_return_key_ctx.detection_active = false;
    m_return_key_ctx.detect_cb = NULL;
    m_return_key_ctx.detect_user_data = NULL;
    m_return_key_ctx.detect_pin_count = 0;
}

bool mui_input_return_key_detect_is_active() { return m_return_key_ctx.detection_active; }
