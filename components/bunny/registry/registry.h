#pragma once
#include "../core/capability.h"
#include <cstddef>

namespace bunny {

static constexpr size_t MAX_CAPABILITIES = 32;

/**
 * Registry — central store for all declared capabilities.
 *
 * Capabilities are registered once at startup (in bunny_load_modules).
 * The Registry is then serialized and sent to the motor de procesos as the
 * device's capability manifest.
 *
 * It also serves as the lookup table for the dispatcher (protocol layer)
 * to route incoming commands and sensor read requests.
 */
class Registry {
public:
    static Registry& instance();

    /** Register a capability. Returns false if the registry is full. */
    bool register_capability(ICapability* cap);

    /** Find a capability by name and kind. Returns nullptr if not found. */
    ICapability* find(const char* name, CapabilityKind kind) const;

    size_t       count() const          { return _count; }
    ICapability* at(size_t i)    const  { return (i < _count) ? _caps[i] : nullptr; }

    /**
     * Serialize all capabilities to a JSON object:
     * {
     *   "sensors":  [...],
     *   "commands": [...],
     *   "events":   [...],
     *   "states":   [...]
     * }
     * Returns bytes written (excl. null terminator).
     */
    size_t serialize_capabilities(char* buf, size_t len) const;

private:
    Registry() = default;

    ICapability* _caps[MAX_CAPABILITIES] {};
    size_t       _count {0};
};

} // namespace bunny
