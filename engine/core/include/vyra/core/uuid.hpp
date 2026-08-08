#pragma once

#include "vyra/core/base.hpp"
#include <cstdint>
#include <functional>

namespace vyra {

    class VYRA_API UUID {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID&) = default;

        operator uint64_t() const { return m_UUID; }

    private:
        uint64_t m_UUID;
    };

} // namespace vyra

namespace std {
    template <typename T> struct hash;
    template<>
    struct hash<vyra::UUID> {
        std::size_t operator()(const vyra::UUID& uuid) const {
            return (uint64_t)uuid;
        }
    };
}
