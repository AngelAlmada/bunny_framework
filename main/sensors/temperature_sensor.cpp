#include "bunny.h"

/**
 * Temperature Sensor — Ejemplo en Modo Rápido (BUNNY_READ)
 *
 * Función C++ directa: Retorna el valor numérico en 1 sola línea.
 */

BUNNY_READ(temperature) {
    return analog_read_mapped(34, -10.0, 60.0);
}
