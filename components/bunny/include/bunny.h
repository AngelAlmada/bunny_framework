#pragma once

/**
 * Bunny Framework - Public API
 *
 * Entry points to initialize and run the Bunny runtime on ESP32.
 * No logic lives here — the ESP32 only exposes capabilities and
 * responds to instructions from the backend.
 */

void bunny_begin(void);
void bunny_load_modules(void);
void bunny_loop(void);
