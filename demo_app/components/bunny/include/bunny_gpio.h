#pragma once

namespace bunny {
namespace gpio {

enum class Mode {
    INPUT,
    OUTPUT,
    INPUT_PULLUP,
    INPUT_PULLDOWN
};

/**
 * Configure a GPIO pin with the specified mode.
 * Returns true if successful, false otherwise.
 */
bool configure(int pin, Mode mode);

/**
 * Set the output level of a configured GPIO pin.
 * level: 1 (HIGH) or 0 (LOW).
 */
void write(int pin, int level);

/**
 * Read the input level of a configured GPIO pin.
 * Returns 1 (HIGH) or 0 (LOW).
 */
int read(int pin);

} // namespace gpio
} // namespace bunny
