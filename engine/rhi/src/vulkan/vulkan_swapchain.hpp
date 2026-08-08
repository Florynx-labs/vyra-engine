#pragma once

#include "vyra/rhi/rhi.hpp"
#include "vulkan_context.hpp"
#include "vulkan_device.hpp"

#include <vector>
#include <algorithm>

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
        VkRenderPass GetVkRenderPass() const { return m_RenderPass; }
        VkFramebuffer GetCurrentFramebuffer() const { return m_Framebuffers[m_ImageIndex]; }
        VkCommandBuffer GetCurrentCommandBuffer() const { return m_CommandBuffers[m_CurrentFrame]; }
        VkExtent2D GetExtent() const { return m_Extent; }

    private:
        VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& modes, bool vsync);
        VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

        void CreateImageViews(VkDevice device);
        void CreateRenderPass(VkDevice device);
        void CreateFramebuffers(VkDevice device);
        void CreateCommandPool(VulkanDevice& device);
        void CreateCommandBuffers(VkDevice device);
        void CreateSyncObjects(VkDevice device);

        VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
        VkFormat m_ImageFormat;
        VkExtent2D m_Extent;

        std::vector<VkImage> m_Images;
        std::vector<VkImageView> m_ImageViews;
        std::vector<VkFramebuffer> m_Framebuffers;

        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> m_CommandBuffers;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;

        uint32_t m_CurrentFrame = 0;
        uint32_t m_ImageIndex = 0;
    };

} // namespace vyra::rhi
