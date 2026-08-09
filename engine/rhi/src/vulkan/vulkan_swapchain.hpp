#pragma once

#include "vyra/rhi/rhi.hpp"
#include "vulkan_context.hpp"
#include "vulkan_device.hpp"
#include "vulkan_sync.hpp"

#include <vector>
#include <algorithm>
#include <memory>

namespace vyra::rhi {

    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    class VulkanSwapChain : public RHISwapChain {
    public:
        VulkanSwapChain() = default;
        ~VulkanSwapChain() override;

        bool Init(RHIContext& context, RHIDevice& device, void* surface, const SwapChainCreateInfo& info) override;
        void Shutdown(RHIDevice& device) override;

        bool AcquireNextImage(RHIDevice& device) override;
        bool Present(RHIDevice& device) override;

        void BeginFrame(RHIDevice& device) override;
        void EndFrame(RHIDevice& device) override;

        uint32_t GetWidth() const override { return m_Extent.width; }
        uint32_t GetHeight() const override { return m_Extent.height; }
        uint32_t GetCurrentFrameIndex() const override { return m_CurrentFrame; }

        VkSwapchainKHR GetVkSwapChain() const { return m_SwapChain; }
        RHIRenderPass* GetRenderPass() const { return m_RenderPass.get(); }
        VkFramebuffer GetCurrentFramebuffer() const { return m_Framebuffers[m_ImageIndex]; }
        VkCommandBuffer GetCurrentCommandBuffer() const { return m_CommandBuffers[m_CurrentFrame]; }
        VkExtent2D GetExtent() const { return m_Extent; }

        // Access to RHI abstractions
        RHIFence* GetCurrentFence() const { return m_Fences[m_CurrentFrame].get(); }
        RHISemaphore* GetImageAvailableSemaphore() const { return m_ImageAvailableSemaphores[m_CurrentFrame].get(); }
        RHISemaphore* GetRenderFinishedSemaphore() const { return m_RenderFinishedSemaphores[m_CurrentFrame].get(); }

    private:
        VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& modes, bool vsync);
        VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

        void CreateImageViews(VkDevice device);
        void CreateFramebuffers(VkDevice device);
        void CreateCommandBuffers(VkDevice device);
        void CreateSyncObjects(RHIDevice& device);

        // Validation helpers
        bool ValidateHandle(VkHandle handle, const char* handleName) const;
        void SafeDestroy(VkHandle handle, const char* handleName, VkDevice device);

        VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
        VkFormat m_ImageFormat;
        VkExtent2D m_Extent;

        std::vector<VkImage> m_Images;
        std::vector<VkImageView> m_ImageViews;
        std::vector<VkFramebuffer> m_Framebuffers;

        // Use RHI abstractions
        Scope<RHIRenderPass> m_RenderPass;
        Scope<RHICommandPool> m_CommandPool;
        std::vector<VkCommandBuffer> m_CommandBuffers; // Still need raw for command recording

        std::vector<Scope<RHISemaphore>> m_ImageAvailableSemaphores;
        std::vector<Scope<RHISemaphore>> m_RenderFinishedSemaphores;
        std::vector<Scope<RHIFence>> m_Fences;

        uint32_t m_CurrentFrame = 0;
        uint32_t m_ImageIndex = 0;
        uint32_t m_GraphicsQueueFamilyIndex = 0; // Store for command pool creation
    };

} // namespace vyra::rhi
