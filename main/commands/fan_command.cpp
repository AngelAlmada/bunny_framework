#include "fan_command.h"
#include "bunny_sdk.h"
#include "bunny_gpio.h"
#include <cstring>

/**
 * Fan Command — example capability module.
 *
 * Declares a command that turns a fan relay ON or OFF.
 * The execute hook performs the GPIO write; no logic here.
 *
 * Motor de procesos sends:
 *   { "type": "command", "command": "setFanState", "params": { "state": "ON" } }
 */

static constexpr int FAN_PIN = 5;  // GPIO pin — adjust to your wiring

static bool s_fan_initialized = false;

static void set_fan_hw(bool on) {
    if (!s_fan_initialized) {
        bunny::gpio::configure(FAN_PIN, bunny::gpio::Mode::OUTPUT);
        s_fan_initialized = true;
    }
    bunny::gpio::write(FAN_PIN, on ? 1 : 0);
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
