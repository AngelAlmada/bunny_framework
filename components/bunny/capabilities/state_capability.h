#pragma once
#include "../core/capability.h"
#include <functional>

namespace bunny {

using StateGetFn = std::function<const char*()>;
using StateSetFn = std::function<void(const char*)>;

/**
 * StateCapability — declares an internal device state variable.
 *
 * State is not business logic — it is a typed slot that holds a value
 * the motor de procesos can read or write (e.g. current fan speed, relay state).
 * Getters/setters interact with hardware registers or global variables.
 *
 * Example:
 *   static const char* fan_state = "OFF";
 *
 *   Bunny.state("fanState", STRING)
 *        .description("Current fan relay state")
 *        .tag("actuator")
 *        .build(
 *            []() -> const char* { return fan_state; },
 *            [](const char* v)   { fan_state = v;    }
 *        );
 */
class StateCapability final : public ICapability {
public:
    StateCapability(const char* name, const Metadata& meta,
                    Type state_type,
                    StateGetFn get_fn = nullptr,
                    StateSetFn set_fn = nullptr);

    CapabilityKind  kind()       const override { return CapabilityKind::STATE; }
    const char*     name()       const override { return _name; }
    const Metadata& metadata()   const override { return _meta; }
    size_t          serialize(char* buf, size_t len) const override;

    Type        state_type()          const { return _state_type; }
    const char* get()                 const;
    void        set(const char* val)  const;

private:
    const char* _name;
    Metadata    _meta;
    Type        _state_type;
    StateGetFn  _get_fn;
    StateSetFn  _set_fn;
};

} // namespace bunny
