#pragma once
#include "../core/capability.h"
#include <functional>

namespace bunny {

using EventEmitFn = std::function<void()>;

/**
 * EventCapability — declares an outbound notification capability.
 *
 * Events are emitted by the device to notify the motor de procesos of something
 * that happened (e.g. motion detected, button pressed).
 * The optional on_emit hook is called when Bunny.emit() is invoked,
 * allowing local side-effects (e.g. LED blink) alongside the notification.
 *
 * Example:
 *   Bunny.event("motion_detected")
 *        .description("Triggered when PIR sensor detects movement")
 *        .tag("sensor")
 *        .build();
 *
 *   // Later, when motion is detected:
 *   Bunny.emit("motion_detected");
 */
class EventCapability final : public ICapability {
public:
    EventCapability(const char* name, const Metadata& meta, EventEmitFn on_emit = nullptr);

    CapabilityKind  kind()     const override { return CapabilityKind::EVENT; }
    const char*     name()     const override { return _name; }
    const Metadata& metadata() const override { return _meta; }
    size_t          serialize(char* buf, size_t len) const override;

    /** Trigger the event (sends to motor de procesos + runs optional local hook). */
    void emit() const;

private:
    const char* _name;
    Metadata    _meta;
    EventEmitFn _on_emit;
};

} // namespace bunny
