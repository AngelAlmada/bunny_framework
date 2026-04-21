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

    /** Emit a named event to the backend. */
    void   emit(const char* event_name);

    /** Read the current value of a named sensor. */
    double read(const char* sensor_name);

    // ── Registry access ──────────────────────────────────────────────────────
    Registry& registry() { return Registry::instance(); }

private:
    BunnySDK() = default;
};

} // namespace bunny

/**
 * Global `Bunny` reference — the primary entry point for the SDK.
 *
 *   Bunny.sensor("temperature").description("...").returns(NUMBER).build(...);
 *   Bunny.emit("motion_detected");
 */
extern bunny::BunnySDK& Bunny;
