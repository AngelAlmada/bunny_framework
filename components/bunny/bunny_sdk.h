#pragma once

/**
 * bunny_sdk.h — Main public header for the Bunny Framework SDK.
 *
 * Include this in your module files to declare capabilities using
 * the fluent builder API.
 *
 *   #include "bunny_sdk.h"
 *
 *   void register_my_module() {
 *       Bunny.sensor("temperature")
 *            .description("Temperature in Celsius")
 *            .returns(NUMBER)
 *            .tag("environment")
 *            .build([]() -> double { return 23.5; });
 *   }
 */

#include "types/bunny_types.h"
#include "metadata/metadata.h"
#include "builder/sensor_builder.h"
#include "builder/command_builder.h"
#include "builder/event_builder.h"
#include "builder/state_builder.h"
#include "registry/registry.h"
#include "capabilities/event_capability.h"
#include "capabilities/sensor_capability.h"
#include "capabilities/command_capability.h"
#include "capabilities/state_capability.h"

namespace bunny {

/**
 * BunnySDK — the central runtime object.
 *
 * Accessed via the global `Bunny` reference.
 * Provides:
 *  - fluent builder factory methods (sensor/command/event/state)
 *  - runtime hooks (emit, read)
 *  - access to the central Registry
 *  - lifecycle management (begin / load_modules / loop)
 */
class BunnySDK {
public:
    static BunnySDK& instance();

    // ── Lifecycle ────────────────────────────────────────────────────────────
    void begin();
    void load_modules();
    void loop();

    // ── Fluent capability builders ───────────────────────────────────────────

    /** Start declaring a sensor capability. */
    SensorBuilder  sensor (const char* name);

    /** Start declaring a command capability. */
    CommandBuilder command(const char* name);

    /** Start declaring an event capability. */
    EventBuilder   event  (const char* name);

    /** Start declaring a state variable. */
    StateBuilder   state  (const char* name, Type state_type);

    // ── Runtime hooks (called from hardware ISRs / tasks) ────────────────────

    /** Emit a named event to the motor de procesos. */
    void   emit(const char* event_name);

    /** Read the current value of a named sensor. */
    double read(const char* sensor_name);

    // ── Registry access ──────────────────────────────────────────────────────
    Registry& registry() { return Registry::instance(); }

private:
    BunnySDK() = default;
};

namespace internal {
    using AutoInitFn = void(*)();

    class AutoRegistrar {
    public:
        explicit AutoRegistrar(AutoInitFn fn);
    };

    void run_auto_registrations();

    // ── ActionInvoker Type Adapters for Direct Functions ─────────────────────
    template <typename F>
    struct ActionInvoker;

    // Direct void() function
    template <>
    struct ActionInvoker<void(*)()> {
        static void register_cmd(const char* name, void(*fn)()) {
            BunnySDK::instance().command(name).execute([fn](const Params&) {
                fn();
            });
        }
    };

    // Direct void(const char*) function
    template <>
    struct ActionInvoker<void(*)(const char*)> {
        static void register_cmd(const char* name, void(*fn)(const char*)) {
            BunnySDK::instance().command(name)
                 .param("param", Type::STRING, "Command parameter")
                 .execute([fn](const Params& p) {
                     fn(p.get_first_string());
                 });
        }
    };

    // Direct void(double) function
    template <>
    struct ActionInvoker<void(*)(double)> {
        static void register_cmd(const char* name, void(*fn)(double)) {
            BunnySDK::instance().command(name)
                 .param("value", Type::NUMBER, "Numeric parameter")
                 .execute([fn](const Params& p) {
                     fn(p.get_first_number());
                 });
        }
    };

    // Direct void(int) function
    template <>
    struct ActionInvoker<void(*)(int)> {
        static void register_cmd(const char* name, void(*fn)(int)) {
            BunnySDK::instance().command(name)
                 .param("value", Type::NUMBER, "Integer parameter")
                 .execute([fn](const Params& p) {
                     fn((int)p.get_first_number());
                 });
        }
    };

    // Direct void(bool) function
    template <>
    struct ActionInvoker<void(*)(bool)> {
        static void register_cmd(const char* name, void(*fn)(bool)) {
            BunnySDK::instance().command(name)
                 .param("state", Type::BOOLEAN, "Boolean parameter")
                 .execute([fn](const Params& p) {
                     fn(p.get_first_bool());
                 });
        }
    };

    // Direct void(const Params&) function
    template <>
    struct ActionInvoker<void(*)(const Params&)> {
        static void register_cmd(const char* name, void(*fn)(const Params&)) {
            BunnySDK::instance().command(name).execute(fn);
        }
    };

} // namespace internal

} // namespace bunny

/**
 * Global `Bunny` reference — the primary entry point for the SDK.
 *
 *   Bunny.sensor("temperature").description("...").returns(NUMBER).build(...);
 *   Bunny.emit("motion_detected");
 */
extern bunny::BunnySDK& Bunny;

// ═════════════════════════════════════════════════════════════════════════════
// 🚀 1. MODO RÁPIDO DIRECTO (Máxima Simplicidad, Cero Lambdas)
// ═════════════════════════════════════════════════════════════════════════════

/**
 * BUNNY_ACTION: Declara un comando como una función C++ normal directamente tipada.
 *
 * Ejemplos:
 *   BUNNY_ACTION(clearScreen) {
 *       display_clear();
 *   }
 *
 *   BUNNY_ACTION(setFanState, const char* state) {
 *       digital_write(15, strcmp(state, "ON") == 0 ? HIGH : LOW);
 *   }
 *
 *   BUNNY_ACTION(setDimmer, int level) {
 *       analog_write(18, level);
 *   }
 */
#define BUNNY_ACTION(name, ...) \
    static void _bunny_act_fn_##name(__VA_ARGS__); \
    static void _bunny_act_init_##name() { \
        ::bunny::internal::ActionInvoker<decltype(&_bunny_act_fn_##name)>::register_cmd(#name, &_bunny_act_fn_##name); \
    } \
    static const ::bunny::internal::AutoRegistrar _bunny_auto_act_##name(_bunny_act_init_##name); \
    static void _bunny_act_fn_##name(__VA_ARGS__)

/**
 * BUNNY_READ: Declara un sensor rápido que simplemente retorna un número.
 *
 * Ejemplo:
 *   BUNNY_READ(temperature) {
 *       return analog_read_mapped(34, -10.0, 60.0);
 *   }
 */
#define BUNNY_READ(name) \
    static double _bunny_read_fn_##name(); \
    static void _bunny_read_init_##name() { \
        ::bunny::BunnySDK::instance().sensor(#name).returns(::bunny::Type::NUMBER).build(_bunny_read_fn_##name); \
    } \
    static const ::bunny::internal::AutoRegistrar _bunny_auto_read_##name(_bunny_read_init_##name); \
    static double _bunny_read_fn_##name()

/**
 * BUNNY_TRIGGER: Declara un evento rápido de 1 línea.
 *
 * Ejemplo:
 *   BUNNY_TRIGGER(motionDetected);
 */
#define BUNNY_TRIGGER(name) \
    static void _bunny_trig_init_##name() { \
        ::bunny::BunnySDK::instance().event(#name); \
    } \
    static const ::bunny::internal::AutoRegistrar _bunny_auto_trig_##name(_bunny_trig_init_##name);

// ═════════════════════════════════════════════════════════════════════════════
// 🌟 2. MODO FLUENT (Para metadatos ricos, tags, descripción y validaciones)
// ═════════════════════════════════════════════════════════════════════════════

#define BUNNY_COMMAND(name) \
    static void _bunny_init_cmd_##name(); \
    static const ::bunny::internal::AutoRegistrar _bunny_auto_cmd_##name(_bunny_init_cmd_##name); \
    static void _bunny_init_cmd_##name()

#define BUNNY_SENSOR(name) \
    static void _bunny_init_sensor_##name(); \
    static const ::bunny::internal::AutoRegistrar _bunny_auto_sensor_##name(_bunny_init_sensor_##name); \
    static void _bunny_init_sensor_##name()

#define BUNNY_EVENT(name) \
    static void _bunny_init_event_##name(); \
    static const ::bunny::internal::AutoRegistrar _bunny_auto_event_##name(_bunny_init_event_##name); \
    static void _bunny_init_event_##name()

#define BUNNY_STATE(name) \
    static void _bunny_init_state_##name(); \
    static const ::bunny::internal::AutoRegistrar _bunny_auto_state_##name(_bunny_init_state_##name); \
    static void _bunny_init_state_##name()


