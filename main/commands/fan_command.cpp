#include "fan_command.h"
#include "bunny_sdk.h"
#include <cstring>

/**
 * Fan Command — example capability module.
 *
 * Declares a command that turns a fan relay ON or OFF.
 * The execute hook performs the GPIO write; no logic here.
 *
 * Backend sends:
 *   { "type": "command", "command": "setFanState", "params": { "state": "ON" } }
 */

static constexpr int FAN_PIN = 5;  // GPIO pin — adjust to your wiring

static void set_fan_hw(bool on) {
    // TODO: replace with real gpio_set_level(FAN_PIN, on ? 1 : 0);
    (void)on;
}

void register_fan_command() {
    Bunny.command("setFanState")
         .description("Turn the fan relay ON or OFF")
         .param("state", STRING, "Target state: ON or OFF")
         .affects("fanState")
         .tag("actuator")
         .example("{\"state\": \"ON\"}")
         .execute([](const bunny::Params& p) {
             const char* state = p.get_string("state");
             set_fan_hw(strcmp(state, "ON") == 0);
         });
}
