#pragma once
#include <cstdint>

namespace bunny {

/**
 * Strong type system for Bunny SDK.
 * Used in ParamDef, returns, state types.
 */
enum class Type : uint8_t {
    NUMBER  = 0,
    STRING  = 1,
    BOOLEAN = 2,
    OBJECT  = 3,
    ARRAY   = 4,
    VOID    = 5,
};

inline const char* type_name(Type t) {
    switch (t) {
        case Type::NUMBER:  return "number";
        case Type::STRING:  return "string";
        case Type::BOOLEAN: return "boolean";
        case Type::OBJECT:  return "object";
        case Type::ARRAY:   return "array";
        case Type::VOID:    return "void";
        default:            return "unknown";
    }
}

} // namespace bunny

/* Convenience aliases for the fluent API:
 *   .returns(NUMBER)  .param("x", STRING)  etc.
 */
static constexpr bunny::Type NUMBER  = bunny::Type::NUMBER;
static constexpr bunny::Type STRING  = bunny::Type::STRING;
static constexpr bunny::Type BOOLEAN = bunny::Type::BOOLEAN;
static constexpr bunny::Type OBJECT  = bunny::Type::OBJECT;
static constexpr bunny::Type ARRAY   = bunny::Type::ARRAY;
