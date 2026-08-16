#include "nrf.h"
#include "nrf_nvic.h"

#include "crash_log.h"

/*
 * Fault handlers capture the crash context into the persistent crash log
 * (see debug/crash_log.c) before resetting, so the reason survives a warm
 * reset and can be shown at the next boot / in the About screen.
 *
 * The exception frame pushed by the CPU sits on top of the stack at the
 * moment the handler is entered:
 *   frame[0..3] = r0..r3, frame[4] = r12, frame[5] = lr, frame[6] = pc,
 *   frame[7] = xpsr
 * It is read with a naked trampoline *before* any prologue pushes happen.
 */

/* global + used: referenced from the naked trampolines' inline asm, which
 * must survive LTO renaming */
__attribute__((used)) void crash_fault_common(uint32_t *frame, uint32_t fault_id) {
    uint32_t pc = frame[6];
    uint32_t lr = frame[5];
    uint32_t sp = (uint32_t)frame;
    uint32_t cfsr = SCB->CFSR;
    uint32_t hfsr = SCB->HFSR;
    uint32_t mmfar = SCB->MMFAR;
    uint32_t bfar = SCB->BFAR;
    __disable_irq();
    crash_log_capture(fault_id, pc, lr, sp, cfsr, hfsr, mmfar, bfar);
}

/* naked trampolines: r0 = exception frame pointer, r1 = fault id */
__attribute__((naked)) void HardFault_Handler(void) {
    __asm volatile("mov r0, sp\nmovs r1, #1\nb crash_fault_common\n");
}

__attribute__((naked)) void MemoryManagement_Handler(void) {
    __asm volatile("mov r0, sp\nmovs r1, #2\nb crash_fault_common\n");
}

__attribute__((naked)) void BusFault_Handler(void) {
    __asm volatile("mov r0, sp\nmovs r1, #3\nb crash_fault_common\n");
}

__attribute__((naked)) void UsageFault_Handler(void) {
    __asm volatile("mov r0, sp\nmovs r1, #4\nb crash_fault_common\n");
}

__attribute__((naked)) void NMI_Handler(void) {
    __asm volatile("mov r0, sp\nmovs r1, #5\nb crash_fault_common\n");
}

void DebugMon_Handler(void) {
    /* debug monitor: capture, then wait for the debugger (do not reset) */
    uint32_t *frame = (uint32_t *)__get_MSP();
    crash_fault_common(frame, CRASH_FAULT_HARDFAULT);
    while (1)
        ;
}
