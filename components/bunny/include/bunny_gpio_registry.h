#pragma once
#include <unordered_map>
#include <string>

namespace bunny {
namespace gpio {

class PinRegistry {
public:
    static PinRegistry& instance() {
        static PinRegistry s_instance;
        return s_instance;
    }

    /**
     * Intenta reservar un pin físico para un módulo.
     * Devuelve true si tiene éxito, false si el pin ya estaba reservado.
     */
    bool reserve(int pin, const std::string& owner_tag) {
        if (_registry.count(pin) > 0) {
            return false; // Conflicto detectado
        }
        _registry[pin] = owner_tag;
        return true;
    }

    void release(int pin) {
        _registry.erase(pin);
    }

    bool is_reserved(int pin) const {
        return _registry.count(pin) > 0;
    }

    std::string get_owner(int pin) const {
        auto it = _registry.find(pin);
        return it != _registry.end() ? it->second : "FREE";
    }

private:
    PinRegistry() = default;
    std::unordered_map<int, std::string> _registry;
};

} // namespace gpio
} // namespace bunny
