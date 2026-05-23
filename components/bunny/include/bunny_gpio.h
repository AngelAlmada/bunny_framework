#pragma once
#include <functional>
#include "bunny_gpio_registry.h"

namespace bunny {
namespace gpio {

enum class Mode {
    INPUT,
    OUTPUT,
    INPUT_PULLUP,
    INPUT_PULLDOWN
};

enum class Edge {
    RISING,  // Flanco de subida (Low -> High)
    FALLING, // Flanco de bajada (High -> Low)
    ANY      // Cualquier cambio de estado
};

// ── Digital I/O ──────────────────────────────────────────────────────────────
/**
 * Configura un pin GPIO con el modo especificado.
 * Opcionalmente registra el propietario para evitar colisiones.
 */
bool configure(int pin, Mode mode, const char* owner_tag = "generic");

/**
 * Establece el nivel de salida de un pin GPIO configurado.
 * level: 1 (HIGH) o 0 (LOW).
 */
void write(int pin, int level);

/**
 * Lee el nivel de entrada de un pin GPIO configurado.
 * Devuelve 1 (HIGH) o 0 (LOW).
 */
int read(int pin);

// ── PWM (Control de intensidad, servomotores) ────────────────────────────────
/**
 * Configura un pin para salida PWM.
 * Bajo el capó utiliza el periférico LEDC de ESP-IDF.
 */
bool pwm_configure(int pin, int channel, int frequency_hz = 5000, int resolution_bits = 8);

/**
 * Establece el ciclo de trabajo (duty cycle) de un pin PWM configurado.
 */
void pwm_write(int pin, int duty_cycle);

// ── ADC (Lectura analógica de sensores) ──────────────────────────────────────
/**
 * Configura un pin como entrada analógica (ADC1).
 */
bool adc_configure(int pin);

/**
 * Lee el valor analógico crudo (0 a 4095).
 */
int adc_read_raw(int pin);

/**
 * Lee el voltaje analógico real calibrado (0.0 a 3.3V).
 */
double adc_read_voltage(int pin);

// ── Interrupciones Asíncronas (Inputs basados en eventos) ────────────────────
using InterruptCallback = std::function<void()>;

/**
 * Vincula una función para ejecutarse asíncronamente cuando el pin cambie de estado.
 * Incluye un filtro anti-rebote (debounce) automático para pulsadores físicos.
 */
bool attach_interrupt(int pin, Edge edge, InterruptCallback callback, int debounce_ms = 50);

/**
 * Desvincula la interrupción del pin físico.
 */
void detach_interrupt(int pin);

} // namespace gpio
} // namespace bunny
