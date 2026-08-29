#include "bunny.h"
#include <cstring>

/**
 * Fan Command — Ejemplo en Modo Rápido (BUNNY_ACTION)
 *
 * Directamente tipado en la función: Cero lambdas, cero extracción manual de params.
 */

static constexpr int FAN_PIN = 15;

BUNNY_ACTION(setFanState, const char* state) {
    pin_mode(FAN_PIN, OUTPUT, "fan_relay");
    digital_write(FAN_PIN, strcmp(state, "ON") == 0 ? HIGH : LOW);
}
