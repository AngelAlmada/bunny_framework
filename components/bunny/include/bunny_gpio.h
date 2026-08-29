#pragma once
#include <functional>
#include <algorithm>
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

// ── E/S Digital Simple (Estilo Arduino / Zero-Config) ────────────────────────
/**
 * Configura el modo de un pin (INPUT, OUTPUT, INPUT_PULLUP, INPUT_PULLDOWN).
 */
bool pin_mode(int pin, Mode mode, const char* owner_tag = "generic");
bool configure(int pin, Mode mode, const char* owner_tag = "generic");

/**
 * Escribe un nivel digital (HIGH / 1 o LOW / 0).
 */
void digital_write(int pin, int level);
void write(int pin, int level);

/**
 * Lee el nivel digital del pin (retorna 1 o 0).
 */
int digital_read(int pin);
int read(int pin);

// ── PWM Ultra-Simple (Auto-asignación de canales LEDC) ──────────────────────
/**
 * Emite una señal PWM en el pin especificado (0 a 255).
 * Auto-asigna un canal LEDC libre si el pin no estaba inicializado.
 */
bool analog_write(int pin, int duty_255);

/**
 * Emite una señal PWM por porcentaje (0.0% a 100.0%).
 */
bool pwm_write_percent(int pin, double percent);

/**
 * Configuración avanzada de PWM con frecuencia y resolución personalizadas.
 * Si channel es -1, auto-asigna el primer canal libre.
 */
bool pwm_configure(int pin, int channel = -1, int frequency_hz = 5000, int resolution_bits = 8);
void pwm_write(int pin, int duty_cycle);

// ── ADC Ultra-Simple (Lectura analógica con Auto-Init y Filtro de Ruido) ────
/**
 * Lee el valor analógico entero (0 a 4095).
 * Auto-inicializa el canal ADC1 si no estaba configurado.
 * samples: número de lecturas rápidas promediadas para filtrar ruido (por defecto 4).
 */
int analog_read(int pin, int samples = 4);
int adc_read_raw(int pin);

/**
 * Lee el voltaje analógico en voltios (0.0 a 3.3V).
 */
double analog_read_voltage(int pin, int samples = 4);
double adc_read_voltage(int pin);

/**
 * Lee el valor analógico como porcentaje (0.0% a 100.0%).
 */
double analog_read_percent(int pin, int samples = 4);

/**
 * Lee el valor analógico y lo interpola a un rango personalizado (ej. 0.0 a 80.0 °C).
 */
double analog_read_mapped(int pin, double out_min, double out_max, int samples = 4);

/**
 * Configuración explícita de ADC1 (opcional).
 */
bool adc_configure(int pin);

// ── Interrupciones Asíncronas (Debounce integrado) ──────────────────────────
using InterruptCallback = std::function<void()>;

/**
 * Vincula una interrupción por hardware con filtro anti-rebote (debounce).
 */
bool attach_interrupt(int pin, Edge edge, InterruptCallback callback, int debounce_ms = 50);
void detach_interrupt(int pin);

} // namespace gpio
} // namespace bunny

// ── Constantes y Helpers Globales (Para máxima simplicidad y DX) ────────────
static constexpr auto INPUT          = ::bunny::gpio::Mode::INPUT;
static constexpr auto OUTPUT         = ::bunny::gpio::Mode::OUTPUT;
static constexpr auto INPUT_PULLUP   = ::bunny::gpio::Mode::INPUT_PULLUP;
static constexpr auto INPUT_PULLDOWN = ::bunny::gpio::Mode::INPUT_PULLDOWN;
static constexpr auto PULLUP         = ::bunny::gpio::Mode::INPUT_PULLUP;
static constexpr auto PULLDOWN       = ::bunny::gpio::Mode::INPUT_PULLDOWN;

static constexpr int HIGH = 1;
static constexpr int LOW  = 0;

static constexpr auto RISING  = ::bunny::gpio::Edge::RISING;
static constexpr auto FALLING = ::bunny::gpio::Edge::FALLING;
static constexpr auto CHANGE  = ::bunny::gpio::Edge::ANY;

// Exportar funciones directamente para uso sin prefijo largo
using ::bunny::gpio::pin_mode;
using ::bunny::gpio::digital_write;
using ::bunny::gpio::digital_read;
using ::bunny::gpio::analog_read;
using ::bunny::gpio::analog_read_voltage;
using ::bunny::gpio::analog_read_percent;
using ::bunny::gpio::analog_read_mapped;
using ::bunny::gpio::analog_write;
using ::bunny::gpio::pwm_write_percent;
using ::bunny::gpio::attach_interrupt;
using ::bunny::gpio::detach_interrupt;
