#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Crash/debug facility for diagnosing random resets and hangs.
 *
 * - Every fault (HardFault / MemManage / BusFault / UsageFault / NMI /
 *   SDK assert / APP_ERROR_CHECK) records a crash log into a dedicated
 *   non-zeroed RAM region (.crash_log) before the system resets.
 * - At the next boot the log is still there: the boot code can display it
 *   on screen, and the About screen shows the last reset reason and the
 *   total crash count.
 * - A stack canary below the stack limit detects stack overflows, and a
 *   periodic watermark scan reports peak stack usage.
 * - Optional watchdog (FW_WDT_ENABLE=1) turns hangs into resets with
 *   RESETREAS.DOG set, so "device froze" becomes diagnosable.
 */

#define CRASH_LOG_MAGIC        0x5049584C /* "PIXL" */
#define CRASH_LOG_CANARY_BYTES 64
#define CRASH_LOG_CANARY_PATTERN 0xA5A5A5A5

/* fault ids (kept in the log so the cause survives the reset) */
#define CRASH_FAULT_HARDFAULT   1
#define CRASH_FAULT_MEM_MANAGE  2
#define CRASH_FAULT_BUS_FAULT   3
#define CRASH_FAULT_USAGE_FAULT 4
#define CRASH_FAULT_NMI         5
#define CRASH_FAULT_SDK_ASSERT  6
#define CRASH_FAULT_SDK_ERROR   7
#define CRASH_FAULT_STACK_OVERFLOW 8

/* NOTE: this struct lives in the .noinit region which is shared with the
 * NFC cache; keep it small (region has only ~64 bytes free). */
typedef struct {
    uint32_t magic;              /* CRASH_LOG_MAGIC while a crash is pending */
    uint32_t seq;                /* total crash count (never cleared) */
    /* last crash details */
    uint32_t fault_id;
    uint32_t pc;
    uint32_t lr;
    uint32_t sp;
    uint32_t cfsr;               /* SCB->CFSR */
    uint32_t hfsr;               /* SCB->HFSR */
    uint32_t info;               /* for SDK_ERROR/ASSERT: pointer to error_info_t */
    uint32_t reset_reason;       /* RESETREAS seen at crash time */
    uint32_t stack_usage;        /* bytes of stack used at fault time */
    uint32_t uptime_s;           /* approx. uptime at fault */
    /* boot info (always recorded, survives reset) */
    uint32_t boot_reset_reason;  /* RESETREAS of the current boot */
} crash_log_t;                   /* 52 bytes */

/* pointer to the persistent log region */
crash_log_t *crash_log_get(void);

/* call once at boot, before RESETREAS is cleared: records reset reason,
 * fills the stack canary. Returns true if a crash is pending from the
 * previous session. */
bool crash_log_init(void);

/* capture a fault/crash and arm the pending flag (then reset or halt) */
void crash_log_capture(uint32_t fault_id, uint32_t pc, uint32_t lr, uint32_t sp,
                       uint32_t cfsr, uint32_t hfsr, uint32_t mmfar, uint32_t bfar);

/* capture an SDK assert / APP_ERROR_CHECK fault (extracts line/file) */
void crash_log_capture_error(uint32_t fault_id, uint32_t pc, uint32_t info);

bool crash_log_is_pending(void);
void crash_log_clear(void);

/* update peak stack usage watermark; call periodically from the main loop */
void crash_log_stack_check(void);
uint32_t crash_log_stack_usage(void);

/* human readable (ASCII) helpers */
const char *crash_log_fault_name(uint32_t fault_id);
const char *crash_log_reset_reason_str(uint32_t reset_reason);

#ifdef __cplusplus
}
#endif
