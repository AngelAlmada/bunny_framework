#pragma once
#include "../metadata/metadata.h"
#include "../types/bunny_types.h"
#include "../capabilities/command_capability.h"

namespace bunny {

/**
 * CommandBuilder — fluent API for declaring a command capability.
 *
 * Usage:
 *   Bunny.command("setFanState")
 *        .description("Turn fan ON or OFF")
 *        .param("state", STRING, "ON or OFF")
 *        .affects("fanState")
 *        .execute([](const Params& p) {
 *            gpio_set_level(FAN_PIN, strcmp(p.get_string("state"), "ON") == 0 ? 1 : 0);
 *        });
 */
class CommandBuilder {
public:
    explicit CommandBuilder(const char* name) : _name(name) {}

    CommandBuilder& description(const char* desc) {
        _meta.description = desc;
        return *this;
    }

    CommandBuilder& param(const char* pname, Type type,
                          const char* desc = nullptr, bool required = true) {
        if (_meta.param_count < MAX_PARAMS) {
            _meta.params[_meta.param_count++] = {pname, type, desc, required};
        }
        return *this;
    }

    CommandBuilder& tag(const char* t) {
        if (_meta.tag_count < MAX_TAGS) _meta.tags[_meta.tag_count++] = t;
        return *this;
    }

    CommandBuilder& affects(const char* cap) {
        if (_meta.affects_count < MAX_AFFECTS) _meta.affects[_meta.affects_count++] = cap;
        return *this;
    }

    CommandBuilder& returns(Type t) {
        _meta.returns_type = t;
        return *this;
    }

    CommandBuilder& example(const char* ex) {
        _meta.example = ex;
        return *this;
    }

    /**
     * Finalizes the capability with its execute hook and registers it.
     * Returns a pointer to the allocated capability (owned by the Registry).
     */
    CommandCapability* execute(CommandExecuteFn exec_fn);

private:
    const char* _name {nullptr};
    Metadata    _meta {};
};

} // namespace bunny
