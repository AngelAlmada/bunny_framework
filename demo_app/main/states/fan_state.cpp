#include "fan_state.h"
#include "bunny_sdk.h"

/**
 * Fan State — example capability module.
 *
 * Declares an internal state variable that tracks the fan relay state.
 * The motor de procesos can read and write this state.
 *
 * Note: the setter does NOT contain logic — it only stores the value.
 * The motor de procesos decides WHEN to set it; the setFanState command applies it.
 */

static const char* s_fan_state = "OFF";

void register_fan_state() {
    Bunny.state("fanState", STRING)
         .description("Current fan relay state (ON or OFF)")
         .tag("actuator")
         .build(
             []() -> const char* { return s_fan_state; },
             [](const char* v)   { s_fan_state = v;    }
         );
}
