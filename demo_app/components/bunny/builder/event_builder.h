#pragma once
#include "../metadata/metadata.h"
#include "../types/bunny_types.h"
#include "../capabilities/event_capability.h"

namespace bunny {

/**
 * EventBuilder — fluent API for declaring an event capability.
 *
 * Usage:
 *   Bunny.event("motion_detected")
 *        .description("Triggered when PIR sensor detects movement")
 *        .tag("sensor")
 *        .build();
 *
 *   // Emit at runtime:
 *   Bunny.emit("motion_detected");
 */
class EventBuilder {
public:
    explicit EventBuilder(const char* name) : _name(name) {}

    EventBuilder& description(const char* desc) {
        _meta.description = desc;
        return *this;
    }

    EventBuilder& param(const char* pname, Type type, const char* desc = nullptr) {
        if (_meta.param_count < MAX_PARAMS) {
            _meta.params[_meta.param_count++] = {pname, type, desc, false};
        }
        return *this;
    }

    EventBuilder& tag(const char* t) {
        if (_meta.tag_count < MAX_TAGS) _meta.tags[_meta.tag_count++] = t;
        return *this;
    }

    EventBuilder& example(const char* ex) {
        _meta.example = ex;
        return *this;
    }

    /**
     * Finalizes and registers the event capability.
     * on_emit is an optional local hardware side-effect hook (e.g. blink LED).
     */
    EventCapability* build(EventEmitFn on_emit = nullptr);

private:
    const char* _name {nullptr};
    Metadata    _meta {};
};

} // namespace bunny
