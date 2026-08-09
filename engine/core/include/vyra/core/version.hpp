#pragma once

#include "vyra/core/base.hpp"
#include <string>

namespace vyra {

    class VYRA_API Version {
    public:
        static int GetMajor() { return 0; }
        static int GetMinor() { return 1; }
        static int GetPatch() { return 2; }
        
        static std::string ToString() {
            return std::to_string(GetMajor()) + "." + 
                   std::to_string(GetMinor()) + "." + 
                   std::to_string(GetPatch());
        }
        
        static bool IsAtLeast(int major, int minor, int patch = 0) {
            if (GetMajor() > major) return true;
            if (GetMajor() < major) return false;
            if (GetMinor() > minor) return true;
            if (GetMinor() < minor) return false;
            return GetPatch() >= patch;
        }
    };

} // namespace vyra