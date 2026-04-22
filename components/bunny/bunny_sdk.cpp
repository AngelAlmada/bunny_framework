#include "bunny_sdk.h"
#include "config/config.h"
#include "network/discovery.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>
#include <cstring>

// ── Builder .build() / .execute() implementations ────────────────────────────
// Placed here so all builders share one translation unit and avoid link issues.

namespace bunny {

static const char* TAG = "bunny";

// SensorBuilder
SensorCapability* SensorBuilder::build(SensorReadFn read_fn) {
    auto* cap = new SensorCapability(_name, _meta, std::move(read_fn));
    Registry::instance().register_capability(cap);
    return cap;
}

// CommandBuilder
CommandCapability* CommandBuilder::execute(CommandExecuteFn exec_fn) {
    auto* cap = new CommandCapability(_name, _meta, std::move(exec_fn));
    Registry::instance().register_capability(cap);
    return cap;
}

// EventBuilder
EventCapability* EventBuilder::build(EventEmitFn on_emit) {
    auto* cap = new EventCapability(_name, _meta, std::move(on_emit));
    Registry::instance().register_capability(cap);
    return cap;
}

// StateBuilder
StateCapability* StateBuilder::build(StateGetFn get_fn, StateSetFn set_fn) {
    auto* cap = new StateCapability(_name, _meta, _state_type,
                                    std::move(get_fn), std::move(set_fn));
    Registry::instance().register_capability(cap);
    return cap;
}

// ── BunnySDK ─────────────────────────────────────────────────────────────────

BunnySDK& BunnySDK::instance() {
    static BunnySDK s_instance;
    return s_instance;
}

void BunnySDK::begin() {
    bunny_config_load();
    bunny_discovery_init();
    ESP_LOGI(TAG, "Bunny framework initialized");
}

void BunnySDK::load_modules() {
    // User modules self-register via their register_*() functions.
    // This hook is called after begin() so all config is available.
}

void BunnySDK::loop() {
    ESP_LOGI(TAG, "Bunny is running. Waiting for commands from motor de procesos...");

    // Keep app_main alive and provide a periodic heartbeat in the monitor.
    while (true) {
        ESP_LOGI(TAG, "Heartbeat: Bunny runtime active");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

SensorBuilder  BunnySDK::sensor (const char* name) { return SensorBuilder(name);  }
CommandBuilder BunnySDK::command(const char* name) { return CommandBuilder(name); }
EventBuilder   BunnySDK::event  (const char* name) { return EventBuilder(name);   }
StateBuilder   BunnySDK::state  (const char* name, Type t) { return StateBuilder(name, t); }

void BunnySDK::emit(const char* event_name) {
    auto* cap = Registry::instance().find(event_name, CapabilityKind::EVENT);
    if (cap) static_cast<EventCapability*>(cap)->emit();
    // TODO: send event notification to motor de procesos via network module
}

double BunnySDK::read(const char* sensor_name) {
    auto* cap = Registry::instance().find(sensor_name, CapabilityKind::SENSOR);
    if (cap) return static_cast<SensorCapability*>(cap)->read();
    return 0.0;
}

} // namespace bunny

// ── Global singleton reference ────────────────────────────────────────────────
bunny::BunnySDK& Bunny = bunny::BunnySDK::instance();

// ── C-compatible entry points (called from app_main in bunny_framework.c) ─────
extern "C" {

void bunny_begin(void) {
    Bunny.begin();
}

void bunny_load_modules(void) {
    Bunny.load_modules();
}

void bunny_loop(void) {
    Bunny.loop();
}

} // extern "C"
