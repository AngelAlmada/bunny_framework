#include "display_state.h"
#include "bunny_sdk.h"
#include "display.h"

/**
 * Display States — Monitor display status
 * 
 * Registers all display state capabilities:
 *  - displayStatus (ON/OFF)
 *  - brightness (0-255)
 *  - lastText (last drawn text)
 */

void register_display_states() {
    Bunny.state("displayStatus", STRING)
        .description("Display power state (ON/OFF)")
        .tag("display")
        .tag("power")
        .build(
            bunny::display::get_display_status,
            bunny::display::set_display_status
        );

    Bunny.state("brightness", NUMBER)
        .description("Backlight brightness level (0-255)")
        .tag("display")
        .tag("light")
        .build(
            bunny::display::get_brightness,
            bunny::display::set_brightness
        );

    Bunny.state("lastText", STRING)
        .description("Last text drawn on screen")
        .tag("display")
        .tag("content")
        .build(
            bunny::display::get_last_text,
            bunny::display::set_last_text
        );
}
