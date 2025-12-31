/*============================= stopwatch.c =============================*/
#include "stopwatch.h"
#include "sevenseg4.h"
#include "stm32f4xx_hal.h"

/* ---------------- Button configuration ----------------
 * PA0 active-low with internal pull-up:
 *   released -> HIGH
 *   pressed  -> LOW
 */
#define BTN_PORT        GPIOA
#define BTN_PIN         GPIO_PIN_0
#define LONGPRESS_MS    1500u

/* ---------------- Buzzer configuration ----------------
 * Active buzzer driven directly by PA6 (GPIO output):
 *   buzzer ON  = PA6 HIGH
 *   buzzer OFF = PA6 LOW
 */
#define BUZ_PORT        GPIOA
#define BUZ_PIN         GPIO_PIN_6
#define DONE_BEEP_MS    1000u

/* ---------------- Stopwatch time model ----------------
 * 1ms tick, decrement tenths every 100ms:
 *   100ms => 0.1s => tenths--
 */
#define TENTH_STEP_MS   100u

#define PAUSE_BLINK_MS   500u


/* ---------------- Internal state ---------------- */
static volatile StopwatchState g_state    = SW_IDLE_PAUSED;
static volatile uint16_t       g_tenths   = 1000;   // 100.0
static volatile uint16_t       g_ms_accum = 0;

/* Long-press tracking (press edge comes from EXTI falling) */
static volatile uint8_t  g_btn_tracking = 0;
static volatile uint8_t  g_long_fired   = 0;
static volatile uint16_t g_hold_ms      = 0;

/* DONE latch + buzzer timing */
static volatile uint8_t  g_done_latched = 0;
static volatile uint16_t g_buz_remain_ms = 0;

static volatile uint16_t g_pause_blink_ms = 0;
static volatile uint8_t  g_pause_show     = 1; // 1=show value, 0=blank

/* ---------------- Local helpers ---------------- */

static inline void buzzer_off(void)
{
  HAL_GPIO_WritePin(BUZ_PORT, BUZ_PIN, GPIO_PIN_RESET);
}

static inline void buzzer_on(void)
{
  HAL_GPIO_WritePin(BUZ_PORT, BUZ_PIN, GPIO_PIN_SET);
}

static void buzzer_beep_start(uint16_t duration_ms)
{
  if (duration_ms == 0)
    return;

  g_buz_remain_ms = duration_ms;
  buzzer_on();
}

static void buzzer_tick_1ms(void)
{
  if (g_buz_remain_ms == 0)
    return;

  g_buz_remain_ms--;

  if (g_buz_remain_ms == 0)
    buzzer_off();
}

static void reset_to_startup(void)
{
  // Startup behavior: 100.0 and paused
  g_state = SW_IDLE_PAUSED;
  g_tenths = 1000;
  g_ms_accum = 0;

  g_pause_blink_ms = 0;
  g_pause_show = 1;

  // Allow DONE beep again after reset
  g_done_latched = 0;

  // Stop buzzer immediately on reset
  g_buz_remain_ms = 0;
  buzzer_off();

  // Update display
  SevenSeg4_SetTenths(g_tenths);
}

static void short_press_toggle(void)
{
  // If we're DONE, ignore short presses (long press still resets)
  if (g_state == SW_DONE)
    return;

  if (g_state == SW_RUNNING)
    g_state = SW_PAUSED;
  else
    g_state = SW_RUNNING;
}


/* ---------------- Public API ---------------- */

void Stopwatch_Init(void)
{
  // Ensure buzzer starts off (PA6 must be configured as output by CubeMX)
  buzzer_off();
  reset_to_startup();
}

void Stopwatch_ButtonEdgePress(void)
{
  // Called from EXTI falling edge AFTER debounce.
  // Begin tracking press duration. Short-press action will occur on release.
  g_btn_tracking = 1;
  g_long_fired   = 0;
  g_hold_ms      = 0;
}

void Stopwatch_Tick1ms(void)
{
  /* --------- 0) Buzzer timing (non-blocking) --------- */
  buzzer_tick_1ms();

  /* --------- 1) Long-press detection (poll while tracking) --------- */
  if (g_btn_tracking)
  {
    GPIO_PinState level = HAL_GPIO_ReadPin(BTN_PORT, BTN_PIN);

    if (level == GPIO_PIN_RESET)
    {
      // still held down
      if (g_hold_ms < 0xFFFF) g_hold_ms++;

      // Fire long-press once
      if (!g_long_fired && (g_hold_ms >= LONGPRESS_MS))
      {
        g_long_fired = 1;
        reset_to_startup();  // long press: reset to 100.0 paused
      }
    }
    else
    {
      // released
      g_btn_tracking = 0;

      // Only do short press if long press didn't fire
      if (!g_long_fired)
      {
        short_press_toggle();
      }
    }
  }

  /* --------- Pause blinking (PAUSED only) --------- */
  if (g_state == SW_PAUSED)
  {
    if (g_pause_blink_ms < 0xFFFF) g_pause_blink_ms++;

    if (g_pause_blink_ms >= PAUSE_BLINK_MS)
    {
      g_pause_blink_ms = 0;
      g_pause_show = (g_pause_show ? 0 : 1);
    }

    if (g_pause_show)
      SevenSeg4_SetTenths(g_tenths);
    else
      SevenSeg4_SetTenths(SEVENSEG4_BLANK);
  }
  else
  {
    // Not in PAUSED: no blink, always show value
    g_pause_blink_ms = 0;
    g_pause_show = 1;
    SevenSeg4_SetTenths(g_tenths);
  }


  /* --------- 2) Stopwatch timekeeping (only when RUNNING) --------- */
  if (g_state == SW_RUNNING)
  {
    if (g_tenths > 0)
    {
      g_ms_accum++;

      if (g_ms_accum >= TENTH_STEP_MS)
      {
        g_ms_accum = 0;
        g_tenths--;

        SevenSeg4_SetTenths(g_tenths);

        // DONE reached: freeze at 000.0, pause, and beep once for 1 second
        if (g_tenths == 0)
        {
          g_state = SW_DONE;

          if (!g_done_latched)
          {
            g_done_latched = 1;
            buzzer_beep_start(DONE_BEEP_MS);
          }
        }
      }
    }
  }
  else
  {
    // Don't carry partial time across pauses
    g_ms_accum = 0;
  }
}

StopwatchState Stopwatch_GetState(void)
{
  return g_state;
}

uint16_t Stopwatch_GetTenths(void)
{
  return g_tenths;
}
