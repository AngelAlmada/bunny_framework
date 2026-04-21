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
        "network/discovery.c"
        # Config
        "config/config.c"
    INCLUDE_DIRS
        "."
        "include"
)
