#include "crash_log.h"

#include <string.h>

#include "nrf.h"
#include "nrf_power.h"

/* linker-provided stack bounds (see application.ld, PROVIDE symbols) */
extern char crash_stack_limit;
extern char crash_stack_top;

/* persistent, non-zeroed region (.noinit, survives warm resets / System OFF) */
static crash_log_t m_crash_log __attribute__((section(".noinit"))) = {0};

/* live peak stack usage (does not need to survive reset) */
static volatile uint32_t m_stack_peak_usage = 0;

crash_log_t *crash_log_get(void) {
    return &m_crash_log;
}

static uint32_t stack_limit_addr(void) {
    return (uint32_t)&crash_stack_limit;
}

static uint32_t stack_top_addr(void) {
    return (uint32_t)&crash_stack_top;
}

/* fill the unused part of the stack with the canary pattern */
static void crash_log_fill_stack_canary(void) {
    uint32_t sp = __get_MSP();
    uint32_t limit = stack_limit_addr();
    if (sp > limit + CRASH_LOG_CANARY_BYTES) {
        /* fill everything below the current SP with the pattern; the top
         * CRASH_LOG_CANARY_BYTES of the pattern act as the overflow canary */
        memset((void *)limit, CRASH_LOG_CANARY_PATTERN & 0xFF, sp - limit);
    }
    m_stack_peak_usage = 0;
}

bool crash_log_init(void) {
    crash_log_t *log = crash_log_get();
    bool pending = (log->magic == CRASH_LOG_MAGIC);

    /* record why THIS boot happened (before anything clears RESETREAS) */
    log->boot_reset_reason = nrf_power_resetreas_get();

    crash_log_fill_stack_canary();
    return pending;
}

void crash_log_capture(uint32_t fault_id, uint32_t pc, uint32_t lr, uint32_t sp,
                       uint32_t cfsr, uint32_t hfsr, uint32_t mmfar, uint32_t bfar) {
    crash_log_t *log = crash_log_get();

    log->magic = CRASH_LOG_MAGIC;
    log->seq++;
    log->fault_id = fault_id;
    log->pc = pc;
    log->lr = lr;
    log->sp = sp;
    log->cfsr = cfsr;
    log->hfsr = hfsr;
    log->info = 0;
    log->reset_reason = nrf_power_resetreas_get();
    log->stack_usage = (sp >= stack_limit_addr()) ? (sp - stack_limit_addr()) : 0;
    /* approximate uptime from RTC1 (app_timer2 clock), prescaler-aware */
    {
        uint32_t prescaler = NRF_RTC1->PRESCALER;
        uint32_t freq = 32768 / (prescaler + 1);
        log->uptime_s = (NRF_RTC1->COUNTER & 0xFFFFFF) / freq;
    }

    /* stack overflow detection: canary below the stack limit corrupted? */
    uint32_t *canary = (uint32_t *)stack_limit_addr();
    uint32_t canary_words = CRASH_LOG_CANARY_BYTES / 4;
    bool overflow = false;
    for (uint32_t i = 0; i < canary_words; i++) {
        if (canary[i] != CRASH_LOG_CANARY_PATTERN) {
            overflow = true;
            break;
        }
    }
    if (overflow) {
        log->fault_id = CRASH_FAULT_STACK_OVERFLOW;
    }
}

void crash_log_capture_error(uint32_t fault_id, uint32_t pc, uint32_t info) {
    /* For SDK asserts/errors, pc points at the faulting instruction; the
     * exception frame / registers are not available here, so keep the
     * caller-supplied PC and a short info dump. */
    uint32_t sp = __get_MSP();
    crash_log_capture(fault_id, pc, 0, sp, 0, 0, 0, 0);
    /* keep the error/assert info pointer (error_info_t/assert_info_t) so it
     * can be dereferenced with a debugger */
    crash_log_get()->info = info;
}

bool crash_log_is_pending(void) {
    return crash_log_get()->magic == CRASH_LOG_MAGIC;
}

void crash_log_clear(void) {
    crash_log_get()->magic = 0;
}

void crash_log_stack_check(void) {
    /* scan from the stack limit upward: the first word that is not the
     * canary pattern marks the deepest stack usage so far */
    uint32_t *p = (uint32_t *)stack_limit_addr();
    uint32_t top = stack_top_addr();
    uint32_t usage = 0;
    while ((uint32_t)p + 4 <= top) {
        if (*p != CRASH_LOG_CANARY_PATTERN) {
            break;
        }
        usage += 4;
        p++;
    }
    if (usage > m_stack_peak_usage) {
        m_stack_peak_usage = usage;
    }
}

uint32_t crash_log_stack_usage(void) {
    crash_log_stack_check();
    return m_stack_peak_usage;
}

const char *crash_log_fault_name(uint32_t fault_id) {
    switch (fault_id) {
    case CRASH_FAULT_HARDFAULT:    return "HARDFAULT";
    case CRASH_FAULT_MEM_MANAGE:   return "MEMMANAGE";
    case CRASH_FAULT_BUS_FAULT:    return "BUSFAULT";
    case CRASH_FAULT_USAGE_FAULT:  return "USAGEFAULT";
    case CRASH_FAULT_NMI:          return "NMI";
    case CRASH_FAULT_SDK_ASSERT:   return "ASSERT";
    case CRASH_FAULT_SDK_ERROR:    return "SDK_ERROR";
    case CRASH_FAULT_STACK_OVERFLOW: return "STACK_OVERFLOW";
    default:                       return "UNKNOWN";
    }
}

const char *crash_log_reset_reason_str(uint32_t rr) {
    /* nRF52 RESETREAS bits */
    if (rr & NRF_POWER_RESETREAS_DOG_MASK)    return "WATCHDOG";
    if (rr & NRF_POWER_RESETREAS_LOCKUP_MASK) return "LOCKUP(FAULT)";
    if (rr & NRF_POWER_RESETREAS_SREQ_MASK)   return "SOFTWARE";
    if (rr & NRF_POWER_RESETREAS_RESETPIN_MASK) return "RESET_PIN";
    if (rr & NRF_POWER_RESETREAS_OFF_MASK)    return "SYSTEM_OFF_WAKE";
    if (rr & NRF_POWER_RESETREAS_NFC_MASK)    return "NFC_WAKE";
    if (rr & NRF_POWER_RESETREAS_LPCOMP_MASK) return "LPCOMP_WAKE";
    if (rr & NRF_POWER_RESETREAS_DIF_MASK)    return "DEBUG";
    return "UNKNOWN";
}
