#include "display_command.h"
#include "bunny_sdk.h"
#include "display.h"

/**
 * Display Commands — Control display actions
 * 
 * Registers all display command capabilities:
 *  - fillScreen(color)
 *  - drawText(text, x, y)
 *  - setBrightness(level)
 *  - clearScreen()
 */

void register_display_commands() {
    Bunny.command("fillScreen")
        .description("Fill entire screen with solid color")
        .param("color", STRING, "Color: black, white, red, green, blue, yellow, cyan, magenta")
        .execute([](const bunny::Params& p) {
            bunny::display::cmd_fill_screen(p);
        });

    Bunny.command("drawText")
        .description("Draw text at specified position on screen")
        .param("text", STRING, "Text to display (max 127 characters)")
        .param("x", NUMBER, "X coordinate in pixels (0-239)")
        .param("y", NUMBER, "Y coordinate in pixels (0-319)")
        .execute([](const bunny::Params& p) {
            bunny::display::cmd_draw_text(p);
        });

    Bunny.command("setBrightness")
        .description("Set backlight brightness level")
        .param("level", NUMBER, "Brightness: 0 (off) to 255 (full)")
        .execute([](const bunny::Params& p) {
            bunny::display::cmd_set_brightness(p);
        });

    Bunny.command("clearScreen")
        .description("Clear screen to black")
        .execute([](const bunny::Params& p) {
            bunny::display::cmd_clear_screen(p);
        });
}
