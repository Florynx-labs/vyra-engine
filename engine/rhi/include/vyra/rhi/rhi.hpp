#pragma once

#include "vyra/core/base.hpp"
#include "vyra/rhi/rhi_resources.hpp"
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

    // -----------------------------------------------------------------------
    // Render Pass Abstraction
    // -----------------------------------------------------------------------
    struct AttachmentDescription {
        uint32_t Format{ 0 }; // VkFormat value
        uint32_t Samples{ 1 }; // VkSampleCountFlagBits
        uint32_t LoadOp{ 0 }; // VkAttachmentLoadOp
        uint32_t StoreOp{ 0 }; // VkAttachmentStoreOp
        uint32_t StencilLoadOp{ 0 }; // VkAttachmentLoadOp
        uint32_t StencilStoreOp{ 0 }; // VkAttachmentStoreOp
        uint32_t InitialLayout{ 0 }; // VkImageLayout
        uint32_t FinalLayout{ 0 }; // VkImageLayout
    };

    struct SubpassDescription {
        uint32_t ColorAttachmentCount{ 0 };
        uint32_t* ColorAttachmentRefs{ nullptr }; // VkAttachmentReference
        uint32_t DepthStencilAttachmentRef{ 0 }; // VkAttachmentReference (as uint32_t for simplicity)
    };

    struct RenderPassCreateInfo {
        AttachmentDescription ColorAttachment;
        AttachmentDescription DepthAttachment;
        SubpassDescription Subpass;
    };

    class VYRA_API RHIRenderPass {
    public:
        virtual ~RHIRenderPass() = default;

        virtual bool Init(RHIDevice& device, const RenderPassCreateInfo& info) = 0;
        virtual void Shutdown(RHIDevice& device) = 0;

        virtual void* GetNativeRenderPass() const = 0;

        static Scope<RHIRenderPass> Create();
    };

    // -----------------------------------------------------------------------
    // Command Buffer Abstraction
    // -----------------------------------------------------------------------
    class VYRA_API RHICommandPool {
    public:
        virtual ~RHICommandPool() = default;

        virtual bool Init(RHIDevice& device, uint32_t queueFamilyIndex) = 0;
        virtual void Shutdown(RHIDevice& device) = 0;

        virtual void* GetNativeCommandPool() const = 0;

        static Scope<RHICommandPool> Create();
    };

    class VYRA_API RHICommandBuffer {
    public:
        virtual ~RHICommandBuffer() = default;

        virtual bool Begin() = 0;
        virtual void End() = 0;
        virtual void Reset() = 0;

        virtual void* GetNativeCommandBuffer() const = 0;

        static Scope<RHICommandBuffer> Create();
    };

    // -----------------------------------------------------------------------
    // Synchronization Primitives
    // -----------------------------------------------------------------------
    class VYRA_API RHIFence {
    public:
        virtual ~RHIFence() = default;

        virtual bool Init(RHIDevice& device, bool signaled = false) = 0;
        virtual void Shutdown(RHIDevice& device) = 0;

        virtual bool Wait(RHIDevice& device, uint64_t timeout) = 0;
        virtual void Reset(RHIDevice& device) = 0;

        virtual void* GetNativeFence() const = 0;

        static Scope<RHIFence> Create();
    };

    class VYRA_API RHISemaphore {
    public:
        virtual ~RHISemaphore() = default;

        virtual bool Init(RHIDevice& device) = 0;
        virtual void Shutdown(RHIDevice& device) = 0;

        virtual void* GetNativeSemaphore() const = 0;

        static Scope<RHISemaphore> Create();
    };

} // namespace vyra::rhi
