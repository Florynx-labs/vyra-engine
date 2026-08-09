#pragma once

#include "vyra/core/base.hpp"
#include <string>
#include <optional>

namespace vyra::rhi {

    // RHI error codes
    enum class RHIErrorCode {
        Success = 0,
        // Vulkan errors
        ErrorOutOfHostMemory = -1,
        ErrorOutOfDeviceMemory = -2,
        ErrorInitializationFailed = -3,
        ErrorDeviceLost = -4,
        ErrorMemoryMapFailed = -5,
        ErrorLayerNotPresent = -6,
        ErrorExtensionNotPresent = -7,
        ErrorFeatureNotPresent = -8,
        ErrorIncompatibleDriver = -9,
        ErrorTooManyObjects = -10,
        ErrorFormatNotSupported = -11,
        ErrorFragmentedPool = -12,
        ErrorUnknown = -13,
        ErrorSurfaceLostKHR = -1000000000,
        ErrorNativeWindowInUseKHR = -1000000001,
        // RHI-specific errors
        ErrorInvalidParameter = -100,
        ErrorNullHandle = -101,
        ErrorInvalidState = -102,
        ErrorOutOfMemory = -103,
        ErrorNotInitialized = -104,
        ErrorAlreadyInitialized = -105,
        ErrorUnsupported = -106,
        ErrorInternal = -107,
    };

    // RHI result type for error handling
    class RHIResult {
    public:
        RHIResult() : m_Code(RHIErrorCode::Success) {}
        RHIResult(RHIErrorCode code) : m_Code(code) {}
        RHIResult(RHIErrorCode code, const std::string& message) 
            : m_Code(code), m_Message(message) {}

        // Success check
        bool IsSuccess() const { return m_Code == RHIErrorCode::Success; }
        bool IsError() const { return m_Code != RHIErrorCode::Success; }

        // Error code access
        RHIErrorCode GetCode() const { return m_Code; }

        // Message access
        const std::string& GetMessage() const { return m_Message; }
        bool HasMessage() const { return !m_Message.empty(); }

        // Create error result
        static RHIResult Error(RHIErrorCode code, const std::string& message = "") {
            return RHIResult(code, message);
        }

        // Create success result
        static RHIResult Success() {
            return RHIResult(RHIErrorCode::Success);
        }

        // Convert Vulkan result to RHI result
        static RHIResult FromVkResult(VkResult vkResult, const std::string& context = "");

    private:
        RHIErrorCode m_Code;
        std::string m_Message;
    };

    // Inline conversion helper
    inline RHIResult RHIResult::FromVkResult(VkResult vkResult, const std::string& context) {
        RHIErrorCode code;
        switch (vkResult) {
            case VK_SUCCESS:
                return RHIResult::Success();
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                code = RHIErrorCode::ErrorOutOfHostMemory;
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                code = RHIErrorCode::ErrorOutOfDeviceMemory;
                break;
            case VK_ERROR_INITIALIZATION_FAILED:
                code = RHIErrorCode::ErrorInitializationFailed;
                break;
            case VK_ERROR_DEVICE_LOST:
                code = RHIErrorCode::ErrorDeviceLost;
                break;
            case VK_ERROR_MEMORY_MAP_FAILED:
                code = RHIErrorCode::ErrorMemoryMapFailed;
                break;
            case VK_ERROR_LAYER_NOT_PRESENT:
                code = RHIErrorCode::ErrorLayerNotPresent;
                break;
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                code = RHIErrorCode::ErrorExtensionNotPresent;
                break;
            case VK_ERROR_FEATURE_NOT_PRESENT:
                code = RHIErrorCode::ErrorFeatureNotPresent;
                break;
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                code = RHIErrorCode::ErrorIncompatibleDriver;
                break;
            case VK_ERROR_TOO_MANY_OBJECTS:
                code = RHIErrorCode::ErrorTooManyObjects;
                break;
            case VK_ERROR_FORMAT_NOT_SUPPORTED:
                code = RHIErrorCode::ErrorFormatNotSupported;
                break;
            case VK_ERROR_FRAGMENTED_POOL:
                code = RHIErrorCode::ErrorFragmentedPool;
                break;
            case VK_ERROR_SURFACE_LOST_KHR:
                code = RHIErrorCode::ErrorSurfaceLostKHR;
                break;
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
                code = RHIErrorCode::ErrorNativeWindowInUseKHR;
                break;
            default:
                code = RHIErrorCode::ErrorUnknown;
                break;
        }

        std::string message = context.empty() ? "" : context + ": ";
        message += "Vulkan error " + std::to_string(static_cast<int>(vkResult));
        return RHIResult::Error(code, message);
    }

} // namespace vyra::rhi