#pragma once

#define VK_NO_PROTOTYPES
#include <volk.h>
#include "vyra/rhi/rhi.hpp"

namespace vyra::rhi {

    // -----------------------------------------------------------------------
    // VulkanRenderPass
    // -----------------------------------------------------------------------
    class VulkanRenderPass : public RHIRenderPass {
    public:
        VulkanRenderPass() = default;
        ~VulkanRenderPass() override = default;

        bool Init(RHIDevice& device, const RenderPassCreateInfo& info) override;
        void Shutdown(RHIDevice& device) override;

        void* GetNativeRenderPass() const override { return (void*)m_RenderPass; }

        VkRenderPass GetVkRenderPass() const { return m_RenderPass; }

    private:
        VkRenderPass m_RenderPass{ VK_NULL_HANDLE };
    };

    // -----------------------------------------------------------------------
    // VulkanCommandPool
    // -----------------------------------------------------------------------
    class VulkanCommandPool : public RHICommandPool {
    public:
        VulkanCommandPool() = default;
        ~VulkanCommandPool() override = default;

        bool Init(RHIDevice& device, uint32_t queueFamilyIndex) override;
        void Shutdown(RHIDevice& device) override;

        void* GetNativeCommandPool() const override { return (void*)m_CommandPool; }

        VkCommandPool GetVkCommandPool() const { return m_CommandPool; }

    private:
        VkCommandPool m_CommandPool{ VK_NULL_HANDLE };
    };

    // -----------------------------------------------------------------------
    // VulkanCommandBuffer
    // -----------------------------------------------------------------------
    class VulkanCommandBuffer : public RHICommandBuffer {
    public:
        VulkanCommandBuffer() = default;
        ~VulkanCommandBuffer() override = default;

        bool Begin() override;
        void End() override;
        void Reset() override;

        void* GetNativeCommandBuffer() const override { return (void*)m_CommandBuffer; }

        VkCommandBuffer GetVkCommandBuffer() const { return m_CommandBuffer; }
        void SetVkCommandBuffer(VkCommandBuffer cmd) { m_CommandBuffer = cmd; }

    private:
        VkCommandBuffer m_CommandBuffer{ VK_NULL_HANDLE };
    };

    // -----------------------------------------------------------------------
    // VulkanFence
    // -----------------------------------------------------------------------
    class VulkanFence : public RHIFence {
    public:
        VulkanFence() = default;
        ~VulkanFence() override = default;

        bool Init(RHIDevice& device, bool signaled = false) override;
        void Shutdown(RHIDevice& device) override;

        bool Wait(RHIDevice& device, uint64_t timeout) override;
        void Reset(RHIDevice& device) override;

        void* GetNativeFence() const override { return (void*)m_Fence; }

        VkFence GetVkFence() const { return m_Fence; }

    private:
        VkFence m_Fence{ VK_NULL_HANDLE };
    };

    // -----------------------------------------------------------------------
    // VulkanSemaphore
    // -----------------------------------------------------------------------
    class VulkanSemaphore : public RHISemaphore {
    public:
        VulkanSemaphore() = default;
        ~VulkanSemaphore() override = default;

        bool Init(RHIDevice& device) override;
        void Shutdown(RHIDevice& device) override;

        void* GetNativeSemaphore() const override { return (void*)m_Semaphore; }

        VkSemaphore GetVkSemaphore() const { return m_Semaphore; }

    private:
        VkSemaphore m_Semaphore{ VK_NULL_HANDLE };
    };

} // namespace vyra::rhi