# Read user config at build-time so firmware defaults match config/device.json.
file(READ "${PROJECT_DIR}/config/device.json" BUNNY_DEVICE_JSON)
string(JSON BUNNY_DEVICE_ID GET "${BUNNY_DEVICE_JSON}" device id)
string(JSON BUNNY_DEVICE_NAME GET "${BUNNY_DEVICE_JSON}" device name)
string(JSON BUNNY_DEVICE_DESCRIPTION GET "${BUNNY_DEVICE_JSON}" device description)
string(JSON BUNNY_DEVICE_TYPE GET "${BUNNY_DEVICE_JSON}" device type)
string(JSON BUNNY_DEVICE_VERSION GET "${BUNNY_DEVICE_JSON}" device version)
string(JSON BUNNY_WIFI_SSID GET "${BUNNY_DEVICE_JSON}" network wifi ssid)
string(JSON BUNNY_WIFI_PASSWORD GET "${BUNNY_DEVICE_JSON}" network wifi password)
string(JSON BUNNY_WIFI_AUTH_TYPE GET "${BUNNY_DEVICE_JSON}" network wifi auth_type)
string(JSON BUNNY_WIFI_MAX_RETRIES GET "${BUNNY_DEVICE_JSON}" network wifi max_retries)
string(JSON BUNNY_WIFI_TIMEOUT_MS GET "${BUNNY_DEVICE_JSON}" network wifi timeout_ms)
string(JSON BUNNY_DISCOVERY_ENABLED GET "${BUNNY_DEVICE_JSON}" discovery enabled)
string(JSON BUNNY_DISCOVERY_UDP_PORT GET "${BUNNY_DEVICE_JSON}" discovery udp_port)
string(JSON BUNNY_DISCOVERY_BROADCAST_INTERVAL_MS GET "${BUNNY_DEVICE_JSON}" discovery broadcast_interval_ms)
string(JSON BUNNY_WEBHOOK_PORT GET "${BUNNY_DEVICE_JSON}" webhook port)
string(JSON BUNNY_WEBHOOK_PATH GET "${BUNNY_DEVICE_JSON}" webhook path)

if(BUNNY_DISCOVERY_ENABLED)
    set(BUNNY_DISCOVERY_ENABLED_C 1)
else()
    set(BUNNY_DISCOVERY_ENABLED_C 0)
endif()

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
        "protocol/protocol.cpp"
        "network/network.c"
        "network/wifi.c"
        "network/discovery.c"
        "platform/esp32/bunny_gpio.cpp"
        # Config
        "config/config.c"
    INCLUDE_DIRS
        "."
        "include"
    REQUIRES
        esp_http_server
        esp_timer
        esp_netif
        esp_wifi
        esp_event
        nvs_flash
        lwip
        esp_driver_gpio
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    BUNNY_DEVICE_ID="${BUNNY_DEVICE_ID}"
    BUNNY_DEVICE_NAME="${BUNNY_DEVICE_NAME}"
    BUNNY_DEVICE_DESCRIPTION="${BUNNY_DEVICE_DESCRIPTION}"
    BUNNY_DEVICE_TYPE="${BUNNY_DEVICE_TYPE}"
    BUNNY_DEVICE_VERSION="${BUNNY_DEVICE_VERSION}"
    BUNNY_WIFI_SSID="${BUNNY_WIFI_SSID}"
    BUNNY_WIFI_PASSWORD="${BUNNY_WIFI_PASSWORD}"
    BUNNY_WIFI_AUTH_TYPE="${BUNNY_WIFI_AUTH_TYPE}"
    BUNNY_WIFI_MAX_RETRIES=${BUNNY_WIFI_MAX_RETRIES}
    BUNNY_WIFI_TIMEOUT_MS=${BUNNY_WIFI_TIMEOUT_MS}
    BUNNY_DISCOVERY_ENABLED=${BUNNY_DISCOVERY_ENABLED_C}
    BUNNY_DISCOVERY_UDP_PORT=${BUNNY_DISCOVERY_UDP_PORT}
    BUNNY_DISCOVERY_BROADCAST_INTERVAL_MS=${BUNNY_DISCOVERY_BROADCAST_INTERVAL_MS}
    BUNNY_WEBHOOK_PORT=${BUNNY_WEBHOOK_PORT}
    BUNNY_WEBHOOK_PATH="${BUNNY_WEBHOOK_PATH}"
)
