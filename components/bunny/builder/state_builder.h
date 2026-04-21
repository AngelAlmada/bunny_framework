#pragma once
#include "../metadata/metadata.h"
#include "../types/bunny_types.h"
#include "../capabilities/state_capability.h"

namespace bunny {

/**
 * StateBuilder — fluent API for declaring a state capability.
 *
 * Usage:
 *   static const char* s_fan = "OFF";
 *
 *   Bunny.state("fanState", STRING)
 *        .description("Current fan relay state (ON/OFF)")
 *        .tag("actuator")
 *        .build(
 *            []() -> const char* { return s_fan; },
 *            [](const char* v)   { s_fan = v;    }
 *        );
 */
class StateBuilder {
public:
    StateBuilder(const char* name, Type state_type)
        : _name(name), _state_type(state_type) {
        _meta.returns_type = state_type;
    }

    StateBuilder& description(const char* desc) {
        _meta.description = desc;
        return *this;
    }

    StateBuilder& tag(const char* t) {
        if (_meta.tag_count < MAX_TAGS) _meta.tags[_meta.tag_count++] = t;
        return *this;
    }

    StateBuilder& example(const char* ex) {
        _meta.example = ex;
        return *this;
    }

    /**
     * Finalizes and registers the state capability.
     * get_fn / set_fn are optional hardware-level accessors.
     */
    StateCapability* build(StateGetFn get_fn = nullptr, StateSetFn set_fn = nullptr);

private:
    const char* _name       {nullptr};
    Type        _state_type {Type::STRING};
    Metadata    _meta       {};
};

} // namespace bunny
