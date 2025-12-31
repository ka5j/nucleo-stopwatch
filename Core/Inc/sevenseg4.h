/*=========================== sevenseg4.h ===========================*/
#ifndef SEVENSEG4_H
#define SEVENSEG4_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define SEVENSEG4_BLANK  0xFFFFu

/*
 * 4-digit 7-seg common-anode multiplex DISPLAY DRIVER for STM32F446.
 *
 * Wiring assumed:
 *   Digits (common anodes, ON=HIGH): PC0, PC1, PC2, PC3
 *   Segments (active LOW, ON=LOW):  PB0(A), PB1(B), PB2(C), PB4(D),
 *                                  PB5(E), PB6(F), PB7(G), PB8(DP)
 *
 * Display format:
 *   HSS.s   (decimal point between digit2 and digit3)
 * Examples:
 *   100.0, 099.9, 042.3, 007.1, 000.0
 *
 * Timing model:
 *   - Call SevenSeg4_Tick1ms() from TIM2 ISR at ~1kHz (1ms)
 *   - This module ONLY multiplexes and displays the current tenths value.
 *   - Stopwatch/timekeeping is handled in stopwatch.c
 */

#ifdef __cplusplus
extern "C" {
#endif

// Initialize display driver state and blank safely.
// Default shown value is 100.0 until app sets something else.
void SevenSeg4_Init(void);

// Set display value in tenths of seconds (0..1000). (1000 = 100.0)
void SevenSeg4_SetTenths(uint16_t tenths);

// Call from TIM2 ISR every 1ms to multiplex the 4 digits
void SevenSeg4_Tick1ms(void);

#ifdef __cplusplus
}
#endif

#endif // SEVENSEG4_H
