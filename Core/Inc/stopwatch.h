/*============================= stopwatch.h =============================*/
#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Stopwatch application logic (includes button + buzzer behavior).
 *
 * Hardware assumptions:
 *  - Button on PA0:
 *      - internal pull-up enabled
 *      - active-low (pressed = 0)
 *      - EXTI on FALLING edge calls Stopwatch_ButtonEdgePress() (after debounce)
 *
 *  - Active buzzer on PA6:
 *      - PA6 configured as GPIO output (push-pull)
 *      - buzzer + -> PA6 (via ~330Ω), buzzer - -> GND
 *      - buzzer ON = PA6 HIGH, buzzer OFF = PA6 LOW
 *
 * Behavior:
 *  - Startup: show 100.0 and PAUSED
 *  - Short press (tap):
 *      RUNNING -> PAUSED
 *      PAUSED/IDLE -> RUNNING
 *  - Long press (hold >= 1.5s): reset to 100.0 and PAUSED (same as startup)
 *  - DONE (reaches 000.0): beep buzzer ON for 1 second ONCE, remain at 000.0 paused
 *
 * Integration:
 *  - Call Stopwatch_Init() once after SevenSeg4_Init()
 *  - Call Stopwatch_Tick1ms() every 1ms (e.g., in TIM2 ISR)
 *  - Call Stopwatch_ButtonEdgePress() from EXTI callback after debounce
 */

typedef enum {
  SW_IDLE_PAUSED = 0,
  SW_RUNNING     = 1,
  SW_PAUSED      = 2,
  SW_DONE        = 3
} StopwatchState;


void Stopwatch_Init(void);
void Stopwatch_Tick1ms(void);
void Stopwatch_ButtonEdgePress(void);

// Optional helpers (debug)
StopwatchState Stopwatch_GetState(void);
uint16_t Stopwatch_GetTenths(void);

#ifdef __cplusplus
}
#endif

#endif /* STOPWATCH_H */
