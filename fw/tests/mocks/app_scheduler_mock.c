#include "app_scheduler.h"

/* Execute scheduled events synchronously - good enough for unit tests */
void app_sched_event_put(void const *p_event_data, uint16_t event_size,
                         app_sched_event_handler_t handler) {
    handler((void *)(uintptr_t)p_event_data, event_size);
}
