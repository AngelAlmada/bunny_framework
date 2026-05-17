#pragma once
#include "../metadata/metadata.h"
#include "../types/bunny_types.h"
#include "../capabilities/sensor_capability.h"

namespace bunny {

/**
 * SensorBuilder — fluent API for declaring a sensor capability.
 *
 * Usage:
 *   Bunny.sensor("temperature")
 *        .description("Temperature in Celsius")
 *        .returns(NUMBER)
 *        .tag("environment")
 *        .build([]() -> double { return read_temperature(); });
 */
class SensorBuilder {
public:
    explicit SensorBuilder(const char* name) : _name(name) {}

    SensorBuilder& description(const char* desc) {
        _meta.description = desc;
        return *this;
    }

    SensorBuilder& returns(Type t) {
        _meta.returns_type = t;
        return *this;
    }

    SensorBuilder& tag(const char* t) {
        if (_meta.tag_count < MAX_TAGS) _meta.tags[_meta.tag_count++] = t;
        return *this;
    }

    SensorBuilder& affects(const char* cap) {
        if (_meta.affects_count < MAX_AFFECTS) _meta.affects[_meta.affects_count++] = cap;
        return *this;
    }

    SensorBuilder& example(const char* ex) {
        _meta.example = ex;
        return *this;
    }

    /**
     * Finalizes the capability with its read hook and registers it.
     * Returns a pointer to the allocated capability (owned by the Registry).
     */
    SensorCapability* build(SensorReadFn read_fn);

private:
    const char* _name {nullptr};
    Metadata    _meta {};
};

} // namespace bunny
