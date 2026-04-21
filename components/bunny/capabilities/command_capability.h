#pragma once
#include "../core/capability.h"
#include <functional>
#include <cstdlib>
#include <cstring>
#include <cstdio>

namespace bunny {

/**
 * Runtime command parameter container.
 *
 * Populated by the protocol layer from the incoming JSON command message.
 * Provides typed accessors; no business logic.
 *
 * Example JSON input:
 *   { "type": "command", "command": "setFanState", "params": { "state": "ON" } }
 */
class Params {
public:
    static constexpr size_t MAX_ENTRIES = 8;

    struct Entry {
        const char* key   {nullptr};
        const char* value {nullptr};  // always stored as string
    };

    void        set(const char* key, const char* value);
    bool        has(const char* key)        const;
    const char* get_string(const char* key) const;
    double      get_number(const char* key) const;
    bool        get_bool(const char* key)   const;

    size_t      size() const { return _count; }

private:
    Entry  _entries[MAX_ENTRIES] {};
    size_t _count {0};

    const Entry* find(const char* key) const;
};

// ─────────────────────────────────────────────────────────────────────────────

using CommandExecuteFn = std::function<void(const Params&)>;

/**
 * CommandCapability — declares an action-executing capability.
 *
 * The execute hook is called when the motor de procesos sends a command message.
 * The lambda receives the typed Params and performs ONLY hardware actions.
 *
 * Example:
 *   Bunny.command("setFanState")
 *        .description("Turn fan ON or OFF")
 *        .param("state", STRING, "ON or OFF")
 *        .affects("fanState")
 *        .execute([](const Params& p) {
 *            gpio_set_level(FAN_PIN, p.get_string("state") == "ON" ? 1 : 0);
 *        });
 */
class CommandCapability final : public ICapability {
public:
    CommandCapability(const char* name, const Metadata& meta, CommandExecuteFn exec_fn);

    CapabilityKind  kind()     const override { return CapabilityKind::COMMAND; }
    const char*     name()     const override { return _name; }
    const Metadata& metadata() const override { return _meta; }
    size_t          serialize(char* buf, size_t len) const override;

    /** Invoke the execute hook with the provided runtime parameters. */
    void execute(const Params& p) const;

private:
    const char*      _name;
    Metadata         _meta;
    CommandExecuteFn _exec_fn;
};

} // namespace bunny
