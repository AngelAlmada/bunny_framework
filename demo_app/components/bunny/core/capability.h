#pragma once
#include "../metadata/metadata.h"
#include <cstddef>

namespace bunny
{

    enum class CapabilityKind : uint8_t
    {
        SENSOR = 0,
        COMMAND = 1,
        EVENT = 2,
        STATE = 3,
    };

    inline const char *capability_kind_name(CapabilityKind k)
    {
        switch (k)
        {
        case CapabilityKind::SENSOR:
            return "sensor";
        case CapabilityKind::COMMAND:
            return "command";
        case CapabilityKind::EVENT:
            return "event";
        case CapabilityKind::STATE:
            return "state";
        default:
            return "unknown";
        }
    }

    /**
     * Abstract base for all Bunny capabilities.
     *
     * A capability is a pure declaration: it describes what the device
     * CAN do or produce, with no business logic attached.
     *
     * serialize() writes a JSON object representation into the provided
     * buffer so the Registry can build the full capabilities payload.
     */
    class ICapability
    {
    public:
        virtual ~ICapability() = default;

        virtual CapabilityKind kind() const = 0;
        virtual const char *name() const = 0;
        virtual const Metadata &metadata() const = 0;

        /** Write JSON representation into buf. Returns bytes written (excl. null). */
        virtual size_t serialize(char *buf, size_t len) const = 0;
    };

} // namespace bunny
