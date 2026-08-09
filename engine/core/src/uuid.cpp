#include "vyra/core/uuid.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <array>

namespace vyra {

    static std::random_device s_RandomDevice;
    static std::mt19937_64 s_Engine(s_RandomDevice());
    static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

    UUID::UUID() {
        // Generate RFC 4122 Version 4 (Random) UUID
        uint64_t randomBytes[2];
        randomBytes[0] = s_UniformDistribution(s_Engine);
        randomBytes[1] = s_UniformDistribution(s_Engine);

        // Copy random bytes to data array
        std::memcpy(m_Data, randomBytes, 16);

        // Set version bits (version 4: 0100xxxx)
        m_Data[6] = (m_Data[6] & 0x0F) | 0x40;
        
        // Set variant bits (variant 1: 10xxxxxx)
        m_Data[8] = (m_Data[8] & 0x3F) | 0x80;
    }

    UUID::UUID(const uint8_t data[16]) {
        std::memcpy(m_Data, data, 16);
    }

    UUID UUID::FromString(const std::string& str) {
        UUID uuid;
        
        // Parse hex string format: "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
        std::string cleanStr;
        for (char c : str) {
            if (c != '-') {
                cleanStr += c;
            }
        }

        if (cleanStr.length() != 32) {
            // Return nil UUID on invalid input
            std::memset(uuid.m_Data, 0, 16);
            return uuid;
        }

        for (size_t i = 0; i < 16; ++i) {
            std::string byteStr = cleanStr.substr(i * 2, 2);
            uuid.m_Data[i] = static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16));
        }

        return uuid;
    }

    std::string UUID::ToString() const {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');

        for (size_t i = 0; i < 16; ++i) {
            if (i == 4 || i == 6 || i == 8 || i == 10) {
                ss << '-';
            }
            ss << std::setw(2) << static_cast<int>(m_Data[i]);
        }

        return ss.str();
    }

    bool UUID::operator==(const UUID& other) const {
        return std::memcmp(m_Data, other.m_Data, 16) == 0;
    }

    bool UUID::operator!=(const UUID& other) const {
        return !(*this == other);
    }

    bool UUID::operator<(const UUID& other) const {
        return std::memcmp(m_Data, other.m_Data, 16) < 0;
    }

    bool UUID::IsNil() const {
        for (size_t i = 0; i < 16; ++i) {
            if (m_Data[i] != 0) {
                return false;
            }
        }
        return true;
    }

} // namespace vyra
