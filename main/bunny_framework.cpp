#include "bunny.h"
#include "sensors/temperature_sensor.h"
#include "commands/fan_command.h"
#include "commands/display_command.h"
#include "events/motion_event.h"
#include "states/fan_state.h"
#include "states/display_state.h"

// Display library (ST7789 2.8" TFT)
#include "display.h"

extern "C" void app_main(void)
{
    bunny_begin();

    /* Initialize display hardware */
    bunny::display::init_hardware();

    /* Register all capabilities — order does not matter */
    register_temperature_sensor();
    register_fan_command();
    register_display_commands();
    register_motion_event();
    register_fan_state();
    register_display_states();

    bunny_load_modules();
    bunny_loop();
}
