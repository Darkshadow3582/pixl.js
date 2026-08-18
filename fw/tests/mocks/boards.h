#ifndef BOARDS_MOCK_H_
#define BOARDS_MOCK_H_

#define BSP_BOARD_LED_0 0

static inline void bsp_board_led_on(uint32_t led) { (void)led; }
static inline void bsp_board_led_off(uint32_t led) { (void)led; }

#endif
