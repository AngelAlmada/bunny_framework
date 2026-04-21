#pragma once
#include "../types/bunny_types.h"
#include <cstddef>

namespace bunny {

static constexpr size_t MAX_PARAMS  = 8;
static constexpr size_t MAX_TAGS    = 8;
static constexpr size_t MAX_AFFECTS = 8;

/**
 * Definition of a single parameter or return value.
 */
struct ParamDef {
    const char* name        {nullptr};
    Type        type        {Type::STRING};
    const char* description {nullptr};
    bool        required    {true};
};

/**
 * Complete metadata for a capability.
 * This is what gets serialized and sent to the motor de procesos so it can
 * reason about which capabilities exist and how to use them.
 *
 * Fields:
 *  description  - human-readable description of the capability
 *  returns_type - type of the value produced (sensors) or returned (commands)
 *  params       - typed input parameter definitions
 *  tags         - semantic labels (e.g. "environment", "actuator")
 *  affects      - other capabilities this one modifies (e.g. command → state)
 *  example      - optional usage example string for LLM consumption
 */
struct Metadata {
    const char* description   {nullptr};
    Type        returns_type  {Type::VOID};

    ParamDef    params[MAX_PARAMS] {};
    size_t      param_count        {0};

    const char* tags[MAX_TAGS] {};
    size_t      tag_count      {0};

    const char* affects[MAX_AFFECTS] {};
    size_t      affects_count        {0};

    const char* example {nullptr};
};

} // namespace bunny
