# STM32F446RE Stopwatch and 4-Digit 7-Segment Display

A fundamentals-focused embedded systems project built on the STM32F446RE Nucleo that implements an interrupt-driven stopwatch using a 4-digit common-anode 7-segment display, a single pushbutton, and an active buzzer.

The project emphasizes timers, interrupts, multiplexing, and state machines rather than feature complexity. All timing and control logic is non-blocking and driven by a single hardware timer interrupt.

---

## Project Overview

This project implements a stopwatch-style countdown timer that starts at **100.0** and counts down to **000.0** with tenths-of-a-second resolution.

Key design goals:
- Learn how real embedded systems manage time without blocking delays
- Practice interrupt-driven design using hardware timers
- Implement a clean finite state machine for user interaction
- Separate hardware drivers from application logic
- Build intuition for display multiplexing and button handling

---

## Features

- 4-digit common-anode 7-segment display with multiplexing  
- Timer-driven system using TIM2 at 1 ms  
- Fully interrupt-driven architecture (empty main loop)  
- Finite state machine with the following states:
  - Idle / Paused (startup)
  - Running
  - Paused
  - Done  
- Single pushbutton interface:
  - Short press: start, pause, resume
  - Long press (≥ 1.5 s): reset to startup state  
- Countdown from **100.0 → 000.0**  
- Display blinks while paused  
- One-shot buzzer beep (1 second) when countdown reaches zero  
- Modular separation between display driver logic, stopwatch logic, and hardware configuration  

---

## System Architecture

### Timing Model

- TIM2 generates a periodic interrupt every **1 ms**
- All system behavior is driven from this interrupt:
  - Display multiplexing
  - Stopwatch timekeeping
  - Button press duration tracking
  - Pause blinking
  - Buzzer timing

No blocking delays (`HAL_Delay`) are used anywhere in the application.

### State Machine

The stopwatch logic is implemented as an explicit finite state machine:

- **IDLE / PAUSED**: Startup state, shows 100.0  
- **RUNNING**: Countdown active  
- **PAUSED**: Countdown frozen, display blinks  
- **DONE**: Countdown reached zero, buzzer beeps once, system paused  

This approach makes behavior predictable and scalable.

---

## Hardware Requirements

- STM32F446RE Nucleo board  
- 4-digit common-anode 7-segment display  
- Momentary pushbutton  
- Active buzzer  
- Breadboard and jumper wires  

---

## 7-Segment Display Details

### Display Type

- Common Anode  
- Digit select: `DIGx = HIGH`  
- Segment ON: `SEGx = LOW`  

### Bit Ordering (Segment Pattern)

| Bit | Segment |
|----:|--------|
| 0 | A |
| 1 | B |
| 2 | C |
| 3 | D |
| 4 | E |
| 5 | F |
| 6 | G |
| 7 | DP |

---

### 7-Segment Display Pinout

| Display Pin | Function |
|------------:|---------|
| 1 | E |
| 2 | D |
| 3 | DP |
| 4 | C |
| 5 | G |
| 6 | DIG4 |
| 7 | B |
| 8 | DIG3 |
| 9 | DIG2 |
| 10 | F |
| 11 | A |
| 12 | DIG1 |

---

### STM32 Pin Mapping

#### Segments

| Segment | STM32 Pin |
|--------:|----------|
| A | PB0 |
| B | PB1 |
| C | PB2 |
| D | PB4 |
| E | PB5 |
| F | PB6 |
| G | PB7 |
| DP | PB8 |

#### Digit Enables

| Digit | STM32 Pin |
|------:|----------|
| DIG1 | PC0 |
| DIG2 | PC1 |
| DIG3 | PC2 |
| DIG4 | PC3 |

---

## Button Connection

- Button pin: **PA0**  
- Internal pull-up enabled  
- Active-low (pressed = 0)  
- EXTI interrupt on falling edge  
- Software debounce and press-duration tracking  

---

## Buzzer Connection

- Buzzer pin: **PA6**  
- Active buzzer driven directly via GPIO  
- One-shot 1 second beep when countdown reaches zero  
- Non-blocking timing logic  

---

## Development Environment

- STM32CubeIDE  
- STM32 HAL drivers  
- STM32CubeMX for pin and peripheral configuration  

---

## Future Improvements

- Further modularization (separate button driver)
- Improved documentation and comments
- Configurable start values
- Multiple countdown or stopwatch modes
- Reimplementation without HAL for lower-level control
- Integration with a simple scheduler or RTOS-style structure

---

## Learning Outcomes

This project reinforced several core embedded systems concepts:
- Why non-blocking, interrupt-driven design matters
- How to manage time using accumulators rather than delays
- Why button handling is more complex than edge detection
- How display multiplexing must be carefully sequenced
- The importance of explicit state machines in firmware design

---

## License

This project is provided for educational purposes.  
STM32 HAL components are subject to STMicroelectronics licensing terms.
