#include "bunny_sdk.h"
#include "config/config.h"
#include "network/discovery.h"
#include "network/network.h"
#include "network/wifi.h"
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
    bunny_network_init();
    ESP_LOGI(TAG, "Bunny framework initialized");
}

void BunnySDK::load_modules() {
    // User modules self-register via their register_*() functions.
    // This hook is called after begin() so all config is available.
    ESP_LOGI(TAG, "Capabilities registered: %u", (unsigned)Registry::instance().count());
    
    // Wait for WiFi connection with timeout from config
    const bunny_config_t* cfg = bunny_config_get();
    uint32_t wifi_timeout = cfg ? cfg->wifi.timeout_ms : 10000;
    
    if (cfg && cfg->wifi.ssid && strlen(cfg->wifi.ssid) > 0) {
        ESP_LOGI(TAG, "Waiting for WiFi connection (timeout: %lums)...", (unsigned long)wifi_timeout);
        if (bunny_wifi_connect_wait(wifi_timeout)) {
            const char* ip = bunny_wifi_get_ip();
            ESP_LOGI(TAG, "WiFi connected! IP: %s", ip ? ip : "unknown");
        } else {
            ESP_LOGW(TAG, "WiFi connection timeout or failed. Check SSID/password.");
        }
    }
    
    bunny_network_connect();
    bunny_discovery_start();
}

void BunnySDK::loop() {
    const bunny_config_t* cfg = bunny_config_get();
    bool stage1_logged = false;
    bool stage2_logged = false;
    bool stage3_logged = false;
    bool stage4_logged = false;

    ESP_LOGI(TAG, "Bunny runtime started");
    ESP_LOGI(TAG, "FLOW[0/4] Waiting WiFi/LAN + motor de procesos");
    if (cfg) {
        ESP_LOGI(TAG,
                 "Device config: id=%s name=%s type=%s version=%s",
                 cfg->device.id,
                 cfg->device.name,
                 cfg->device.type,
                 cfg->device.version);
        ESP_LOGI(TAG,
                 "Network config: udp_port=%u webhook=ws://<device-ip>:%u%s",
                 (unsigned)cfg->discovery.udp_port,
                 (unsigned)cfg->webhook.port,
                 cfg->webhook.path);
    }

    // Keep app_main alive and provide a periodic heartbeat in the monitor.
    while (true) {
        uint32_t announces = bunny_discovery_announce_count();
        bool wifi_connected = bunny_wifi_connected();
        bool ws_connected = bunny_network_ws_connected();
        const char* wifi_ip = bunny_wifi_get_ip();

        if (!stage1_logged && wifi_connected) {
            ESP_LOGI(TAG, "FLOW[1/4] WiFi connected at IP: %s", wifi_ip ? wifi_ip : "unknown");
            stage1_logged = true;
        }

        if (!stage2_logged && announces > 0) {
            ESP_LOGI(TAG, "FLOW[2/4] UDP announce observed in network (%lu sent)", (unsigned long)announces);
            stage2_logged = true;
        }

        if (!stage3_logged && announces > 0 && !ws_connected) {
            ESP_LOGI(TAG, "FLOW[3/4] Waiting for WebSocket handshake from motor de procesos");
            stage3_logged = true;
        }

        if (!stage4_logged && ws_connected) {
            ESP_LOGI(TAG, "FLOW[4/4] WebSocket connection established (real)");
            ESP_LOGI(TAG, "Bunny ready to receive commands and publish events");
            stage4_logged = true;
        }

        ESP_LOGI(TAG,
                 "Heartbeat: wifi=%s announces=%lu websocket=%s",
                 wifi_connected ? "connected" : "waiting",
                 (unsigned long)announces,
                 ws_connected ? "connected" : "waiting");
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
