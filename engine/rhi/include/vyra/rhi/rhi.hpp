#pragma once

#include "vyra/core/base.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace vyra::rhi {

    struct ContextCreateInfo {
        std::string AppName = "VYRA Engine";
        uint32_t AppVersion = 1;
        bool EnableValidation = true;
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> PresentFamily;

        bool IsComplete() const {
            return GraphicsFamily.has_value() && PresentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        // Opaque — populated by the Vulkan backend
        bool Adequate = false;
        uint32_t MinImageCount = 0;
    };

    // Forward declarations for the RHI interface types
    // These are implemented by the Vulkan backend but exposed
    // through the abstract interface so higher layers never see Vulkan types.

    class VYRA_API RHIContext {
    public:
        virtual ~RHIContext() = default;

        virtual bool Init(const ContextCreateInfo& info) = 0;
        virtual void Shutdown() = 0;

        virtual void* GetInstance() const = 0;  // VkInstance under the hood
        virtual bool IsValidationEnabled() const = 0;

        static Scope<RHIContext> Create();
    };

    class VYRA_API RHIDevice {
    public:
        virtual ~RHIDevice() = default;

        virtual bool Init(RHIContext& context, void* surface) = 0;
        virtual void Shutdown() = 0;

        virtual void* GetNativeDevice() const = 0;       // VkDevice
        virtual void* GetPhysicalDevice() const = 0;      // VkPhysicalDevice
        virtual void* GetGraphicsQueue() const = 0;       // VkQueue
        virtual void* GetPresentQueue() const = 0;        // VkQueue
        virtual QueueFamilyIndices GetQueueFamilies() const = 0;

        virtual void WaitIdle() = 0;

        static Scope<RHIDevice> Create();
    };

    struct SwapChainCreateInfo {
        uint32_t Width = 1600;
        uint32_t Height = 900;
        bool VSync = true;
    };

    class VYRA_API RHISwapChain {
    public:
        virtual ~RHISwapChain() = default;

        virtual bool Init(RHIContext& context, RHIDevice& device, void* surface, const SwapChainCreateInfo& info) = 0;
        virtual void Shutdown(RHIDevice& device) = 0;

        virtual bool AcquireNextImage(RHIDevice& device) = 0;
        virtual bool Present(RHIDevice& device) = 0;

        virtual void BeginFrame(RHIDevice& device) = 0;
        virtual void EndFrame(RHIDevice& device) = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetCurrentFrameIndex() const = 0;

        static Scope<RHISwapChain> Create();
    };

} // namespace vyra::rhi
