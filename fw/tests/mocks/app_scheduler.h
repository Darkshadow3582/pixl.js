#ifndef APP_SCHEDULER_MOCK_H_
#define APP_SCHEDULER_MOCK_H_

#include <stdint.h>

typedef void (*app_sched_event_handler_t)(void *p_event_data, uint16_t event_size);

void app_sched_event_put(void const *p_event_data, uint16_t event_size, app_sched_event_handler_t handler);

#endif
