/*=========================== sevenseg4.c ===========================*/
#include "sevenseg4.h"

/* -------------------- Internal state -------------------- */

// Segment LUT:
// index 0 = OFF
// index 1..10 = digits 0..9
static const uint8_t seg_lut[11] = {
  0b00000000, // OFF
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111  // 9
};

// Value shown (tenths of seconds): 0..1000 (1000 = 100.0)
static volatile uint16_t g_disp_tenths = 1000;

// Display buffer: pattern per digit 0..3
static volatile uint8_t disp_buf[4];

// Which digit is being scanned now
static volatile uint8_t scan_idx = 0;

/* -------------------- GPIO helpers -------------------- */

static inline void digits_all_off(void)
{
  // Common anode: OFF = LOW
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
}

static inline void digit_on(uint8_t idx)
{
  // Common anode: ON = HIGH
  switch (idx)
  {
    case 0: HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET); break;
    case 1: HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET); break;
    case 2: HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET); break;
    case 3: HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET); break;
    default: break;
  }
}

static inline void set_segments_all_off(void)
{
  // Common anode: segment OFF = HIGH
  HAL_GPIO_WritePin(GPIOB,
    GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 |
    GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8,
    GPIO_PIN_SET);
}

static inline void set_segments_from_pattern(uint8_t pat)
{
  // Common anode:
  // pat bit = 1 -> segment ON  -> drive LOW
  // pat bit = 0 -> segment OFF -> drive HIGH
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (pat & 0x01) ? GPIO_PIN_RESET : GPIO_PIN_SET); // A
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, (pat & 0x02) ? GPIO_PIN_RESET : GPIO_PIN_SET); // B
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, (pat & 0x04) ? GPIO_PIN_RESET : GPIO_PIN_SET); // C
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, (pat & 0x08) ? GPIO_PIN_RESET : GPIO_PIN_SET); // D
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, (pat & 0x10) ? GPIO_PIN_RESET : GPIO_PIN_SET); // E
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, (pat & 0x20) ? GPIO_PIN_RESET : GPIO_PIN_SET); // F
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, (pat & 0x40) ? GPIO_PIN_RESET : GPIO_PIN_SET); // G
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, (pat & 0x80) ? GPIO_PIN_RESET : GPIO_PIN_SET); // DP
}

static inline void display_scan_step(void)
{
  // Ghosting prevention sequence:
  // 1) digits off
  // 2) segments off
  // 3) set segments for digit
  // 4) enable digit
  digits_all_off();
  set_segments_all_off();

  set_segments_from_pattern(disp_buf[scan_idx]);
  digit_on(scan_idx);

  scan_idx = (scan_idx + 1) & 0x03;
}

/* -------------------- Display formatting -------------------- */

static inline void update_display_from_tenths(uint16_t t)
{
  // Format: HSS.s
  // 1000 -> 100.0
  //  999 -> 099.9
  //   42 -> 004.2
  //    0 -> 000.0

  uint8_t hundreds = (t / 1000) % 10;  // 0..1 for our range
  uint8_t tens     = (t / 100)  % 10;
  uint8_t ones     = (t / 10)   % 10;
  uint8_t tenths   =  t % 10;

  disp_buf[0] = seg_lut[hundreds + 1];
  disp_buf[1] = seg_lut[tens + 1];
  disp_buf[2] = seg_lut[ones + 1] | 0x80; // DP after ones
  disp_buf[3] = seg_lut[tenths + 1];
}

/* -------------------- Public API -------------------- */

void SevenSeg4_Init(void)
{
  scan_idx = 0;

  // Default shown value (until app sets its own)
  g_disp_tenths = 1000;

  digits_all_off();
  set_segments_all_off();

  update_display_from_tenths(g_disp_tenths);
}

void SevenSeg4_SetTenths(uint16_t tenths)
{
  if (tenths == SEVENSEG4_BLANK)
  {
    g_disp_tenths = SEVENSEG4_BLANK;
    return;
  }

  if (tenths > 1000) tenths = 1000;
  g_disp_tenths = tenths;
}


void SevenSeg4_Tick1ms(void)
{
  // Rebuild digits once per frame (prevents tearing)
	if (scan_idx == 0)
	{
	  if (g_disp_tenths == SEVENSEG4_BLANK)
	  {
	    disp_buf[0] = seg_lut[0];
	    disp_buf[1] = seg_lut[0];
	    disp_buf[2] = seg_lut[0];
	    disp_buf[3] = seg_lut[0];
	  }
	  else
	  {
	    update_display_from_tenths(g_disp_tenths);
	  }
	}

  // Multiplex scan step
  display_scan_step();
}
