#pragma once

#include "vyra/core/base.hpp"
#include <string>

namespace vyra {

    class VYRA_API Version {
    public:
        static int GetMajor() { return VYRA_VERSION_MAJOR; }
        static int GetMinor() { return VYRA_VERSION_MINOR; }
        static int GetPatch() { return VYRA_VERSION_PATCH; }
        
        static std::string ToString() {
            return std::to_string(VYRA_VERSION_MAJOR) + "." + 
                   std::to_string(VYRA_VERSION_MINOR) + "." + 
                   std::to_string(VYRA_VERSION_PATCH);
        }
        
        static bool IsAtLeast(int major, int minor, int patch = 0) {
            if (VYRA_VERSION_MAJOR > major) return true;
            if (VYRA_VERSION_MAJOR < major) return false;
            if (VYRA_VERSION_MINOR > minor) return true;
            if (VYRA_VERSION_MINOR < minor) return false;
            return VYRA_VERSION_PATCH >= patch;
        }
    };

} // namespace vyra