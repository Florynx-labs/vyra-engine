#pragma once

#include "vyra/core/base.hpp"
#include <cstdint>
#include <functional>
#include <string>

namespace vyra {

    class VYRA_API UUID {
    public:
        UUID();
        UUID(const uint8_t data[16]);
        UUID(const UUID&) = default;
        UUID& operator=(const UUID&) = default;

        // Create UUID from string (hex format)
        static UUID FromString(const std::string& str);
        
        // Convert to string (hex format)
        std::string ToString() const;

        // Comparison operators
        bool operator==(const UUID& other) const;
        bool operator!=(const UUID& other) const;
        bool operator<(const UUID& other) const;

        // Get raw data
        const uint8_t* GetData() const { return m_Data; }
        
        // Check if UUID is nil (all zeros)
        bool IsNil() const;

    private:
        uint8_t m_Data[16];
    };

} // namespace vyra

namespace std {
    template <typename T> struct hash;
    template<>
    struct hash<vyra::UUID> {
        std::size_t operator()(const vyra::UUID& uuid) const {
            // Hash the 128-bit UUID data
            std::size_t h1 = std::hash<uint64_t>{}(*reinterpret_cast<const uint64_t*>(uuid.GetData()));
            std::size_t h2 = std::hash<uint64_t>{}(*reinterpret_cast<const uint64_t*>(uuid.GetData() + 8));
            return h1 ^ (h2 << 1);
        }
    };
}
