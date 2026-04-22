# Read user config at build-time so firmware defaults match config/device.json.
file(READ "${PROJECT_DIR}/config/device.json" BUNNY_DEVICE_JSON)
string(JSON BUNNY_WIFI_SSID GET "${BUNNY_DEVICE_JSON}" network wifi ssid)
string(JSON BUNNY_WIFI_PASSWORD GET "${BUNNY_DEVICE_JSON}" network wifi password)
string(JSON BUNNY_WIFI_AUTH_TYPE GET "${BUNNY_DEVICE_JSON}" network wifi auth_type)
string(JSON BUNNY_WIFI_MAX_RETRIES GET "${BUNNY_DEVICE_JSON}" network wifi max_retries)
string(JSON BUNNY_WIFI_TIMEOUT_MS GET "${BUNNY_DEVICE_JSON}" network wifi timeout_ms)

idf_component_register(
    SRCS
        # C++ SDK — entry point, builders, registry
        "bunny_sdk.cpp"
        # Capabilities
        "capabilities/sensor_capability.cpp"
        "capabilities/command_capability.cpp"
        "capabilities/event_capability.cpp"
        "capabilities/state_capability.cpp"
        # Registry
        "registry/registry.cpp"
        # Runtime, protocol, network (stubs — to be implemented)
        "runtime/runtime.c"
        "protocol/protocol.c"
        "network/network.c"
        "network/wifi.c"
        "network/discovery.c"
        # Config
        "config/config.c"
    INCLUDE_DIRS
        "."
        "include"
    REQUIRES
        esp_http_server
        esp_netif
        esp_wifi
        esp_event
        nvs_flash
        lwip
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    BUNNY_WIFI_SSID="${BUNNY_WIFI_SSID}"
    BUNNY_WIFI_PASSWORD="${BUNNY_WIFI_PASSWORD}"
    BUNNY_WIFI_AUTH_TYPE="${BUNNY_WIFI_AUTH_TYPE}"
    BUNNY_WIFI_MAX_RETRIES=${BUNNY_WIFI_MAX_RETRIES}
    BUNNY_WIFI_TIMEOUT_MS=${BUNNY_WIFI_TIMEOUT_MS}
)
