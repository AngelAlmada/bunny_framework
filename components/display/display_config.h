#pragma once

/**
 * ST7789 2.8" TFT Display Configuration
 * 
 * Edit these pins to match your hardware wiring.
 * All pins use GPIO numbers (not board pin numbers).
 */

// ─────────────────────────────────────────────────────────────────────────────
// GPIO PIN CONFIGURATION
// ─────────────────────────────────────────────────────────────────────────────

#define DISPLAY_PIN_MOSI    23      // SPI MOSI (Data Out)
#define DISPLAY_PIN_SCLK    18      // SPI Clock
#define DISPLAY_PIN_CS      5       // Chip Select
#define DISPLAY_PIN_DC      2       // Data/Command
#define DISPLAY_PIN_RST     4       // Reset
#define DISPLAY_PIN_MISO    19      // SPI MISO (Data In)
#define DISPLAY_PIN_BL      32      // Backlight (PWM or GPIO)

// ─────────────────────────────────────────────────────────────────────────────
// DISPLAY PARAMETERS
// ─────────────────────────────────────────────────────────────────────────────

#define DISPLAY_WIDTH       240     // Pixel width
#define DISPLAY_HEIGHT      320     // Pixel height
#define DISPLAY_SPI_FREQ    40000000 // 40 MHz SPI frequency

// ─────────────────────────────────────────────────────────────────────────────
// COLOR DEFINITIONS (565 RGB)
// ─────────────────────────────────────────────────────────────────────────────

#define DISPLAY_COLOR_BLACK     0x0000
#define DISPLAY_COLOR_NAVY      0x000F
#define DISPLAY_COLOR_GREEN     0x07E0
#define DISPLAY_COLOR_CYAN      0x07FF
#define DISPLAY_COLOR_RED       0xF800
#define DISPLAY_COLOR_MAGENTA   0xF81F
#define DISPLAY_COLOR_YELLOW    0xFFE0
#define DISPLAY_COLOR_WHITE     0xFFFF

#define DISPLAY_COLOR_DEFAULT_BG    DISPLAY_COLOR_BLACK
#define DISPLAY_COLOR_DEFAULT_TEXT  DISPLAY_COLOR_WHITE
