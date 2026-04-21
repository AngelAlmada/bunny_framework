#pragma once
#include "../core/capability.h"
#include <functional>

namespace bunny {

using SensorReadFn = std::function<double()>;

/**
 * SensorCapability — declares a data-producing capability.
 *
 * The read hook is called by the runtime when the motor de procesos requests
 * a sensor reading. No logic lives here; the lambda only reads hardware.
 *
 * Example:
 *   Bunny.sensor("temperature")
 *        .description("Temperature in Celsius")
 *        .returns(NUMBER)
 *        .tag("environment")
 *        .build([]() -> double { return read_adc_temp(); });
 */
class SensorCapability final : public ICapability {
public:
    SensorCapability(const char* name, const Metadata& meta, SensorReadFn read_fn);

    CapabilityKind  kind()     const override { return CapabilityKind::SENSOR; }
    const char*     name()     const override { return _name; }
    const Metadata& metadata() const override { return _meta; }
    size_t          serialize(char* buf, size_t len) const override;

    /** Invoke the read hook and return the current sensor value. */
    double read() const;

private:
    const char*  _name;
    Metadata     _meta;
    SensorReadFn _read_fn;
};

} // namespace bunny
