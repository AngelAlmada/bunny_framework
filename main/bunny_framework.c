#include "bunny.h"
#include "sensors/temperature_sensor.h"
#include "commands/fan_command.h"
#include "events/motion_event.h"
#include "states/fan_state.h"

void app_main(void)
{
    bunny_begin();

    /* Register all capabilities — order does not matter */
    register_temperature_sensor();
    register_fan_command();
    register_motion_event();
    register_fan_state();

    bunny_load_modules();
    bunny_loop();
}
