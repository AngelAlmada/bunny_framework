#pragma once

#include "bunny_sdk.h"
/**
 * ST7789 2.8" TFT Display Module
 * 
 * Provides hardware initialization and command/state handlers.
 * Capabilities are registered from main() to maintain design pattern consistency.
 */

namespace bunny {
namespace display {

/**
 * Initialize display hardware
 * Call this before registering capabilities
 */
void init_hardware();

/**
 * Get handler functions for capabilities
 * These are used in main() to register commands and states
 */

// State handlers
const char* get_display_status();
void set_display_status(const char* value);

const char* get_brightness();
void set_brightness(const char* value);

const char* get_last_text();
void set_last_text(const char* value);

// Command handlers
void cmd_fill_screen(const Params& params);
void cmd_draw_text(const Params& params);
void cmd_set_brightness(const Params& params);
void cmd_clear_screen(const Params& params);

} // namespace display
} // namespace bunny

