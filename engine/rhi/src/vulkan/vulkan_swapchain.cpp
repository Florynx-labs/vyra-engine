#include "vulkan_swapchain.hpp"
#include "vyra/core/log.hpp"

#include <limits>
#include <array>

namespace vyra::rhi {

    Scope<RHISwapChain> RHISwapChain::Create() {
        return CreateScope<VulkanSwapChain>();
    }

    VulkanSwapChain::~VulkanSwapChain() {
        // Shutdown must be called explicitly with a device reference
    }

    bool VulkanSwapChain::Init(RHIContext& context, RHIDevice& device, void* surface, const SwapChainCreateInfo& info) {
        auto& vkDevice = static_cast<VulkanDevice&>(device);
        VkDevice logicalDevice = vkDevice.GetVkDevice();
        VkPhysicalDevice physicalDevice = vkDevice.GetVkPhysicalDevice();
        VkSurfaceKHR vkSurface = static_cast<VkSurfaceKHR>(surface);

        // Query swap chain support
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vkSurface, &capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vkSurface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vkSurface, &formatCount, formats.data());

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vkSurface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vkSurface, &presentModeCount, presentModes.data());

        if (formats.empty() || presentModes.empty()) {
            VYRA_LOG_CHANNEL(LogChannel::Renderer, error, "Swap chain support is inadequate");
            return false;
        }

        VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(formats);
        VkPresentModeKHR presentMode = ChoosePresentMode(presentModes, info.VSync);
        VkExtent2D extent = ChooseExtent(capabilities, info.Width, info.Height);

        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = vkSurface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = vkDevice.GetQueueFamilies();
        uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };
        if (indices.GraphicsFamily != indices.PresentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VkResult result = vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &m_SwapChain);
        if (result != VK_SUCCESS) {
            VYRA_LOG_CHANNEL(LogChannel::Renderer, error, "Failed to create Vulkan swap chain");
            return false;
        }

        m_ImageFormat = surfaceFormat.format;
        m_Extent = extent;

        // Store graphics queue family index for command pool creation
        m_GraphicsQueueFamilyIndex = vkDevice.GetQueueFamilies().GraphicsFamily.value();

        vkGetSwapchainImagesKHR(logicalDevice, m_SwapChain, &imageCount, nullptr);
        m_Images.resize(imageCount);
        vkGetSwapchainImagesKHR(logicalDevice, m_SwapChain, &imageCount, m_Images.data());

        CreateImageViews(logicalDevice);
        
        // Use RHI render pass abstraction
        m_RenderPass = RHIRenderPass::Create();
        if (m_RenderPass) {
            vyra::rhi::RenderPassCreateInfo rpInfo{};
            rpInfo.ColorAttachment.Format = static_cast<uint32_t>(m_ImageFormat);
            rpInfo.ColorAttachment.Samples = 1;
            rpInfo.ColorAttachment.LoadOp = 2; // VK_ATTACHMENT_LOAD_OP_CLEAR
            rpInfo.ColorAttachment.StoreOp = 1; // VK_ATTACHMENT_STORE_OP_STORE
            rpInfo.ColorAttachment.InitialLayout = 0; // VK_IMAGE_LAYOUT_UNDEFINED
            rpInfo.ColorAttachment.FinalLayout = 5; // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            
            // Note: For simple swapchain, we don't need depth attachment
            rpInfo.DepthAttachment.Format = 0; // VK_FORMAT_UNDEFINED
            rpInfo.DepthAttachment.Samples = 1;
            rpInfo.DepthAttachment.LoadOp = 0; // VK_ATTACHMENT_LOAD_OP_DONT_CARE
            rpInfo.DepthAttachment.StoreOp = 0; // VK_ATTACHMENT_STORE_OP_DONT_CARE
            
            if (!m_RenderPass->Init(device, rpInfo)) {
                VYRA_LOG_ERROR("[VulkanSwapChain] Failed to create render pass");
                return false;
            }
        }
        
        CreateFramebuffers(logicalDevice);
        CreateCommandBuffers(logicalDevice);
        CreateSyncObjects(device);

        VYRA_LOG_CHANNEL(LogChannel::Renderer, info, "Vulkan swap chain created ({}x{}, {} images)", m_Extent.width, m_Extent.height, m_Images.size());
        return true;
    }

    void VulkanSwapChain::Shutdown(RHIDevice& device) {
        auto& vkDevice = static_cast<VulkanDevice&>(device);
        VkDevice logicalDevice = vkDevice.GetVkDevice();
        if (logicalDevice == VK_NULL_HANDLE) return;

        vkDevice.WaitIdle();

        // Destroy RHI synchronization objects first (proper order)
        for (auto& fence : m_Fences) {
            if (fence) fence->Shutdown(device);
        }
        m_Fences.clear();

        for (auto& sem : m_RenderFinishedSemaphores) {
            if (sem) sem->Shutdown(device);
        }
        m_RenderFinishedSemaphores.clear();

        for (auto& sem : m_ImageAvailableSemaphores) {
            if (sem) sem->Shutdown(device);
        }
        m_ImageAvailableSemaphores.clear();

        // Destroy command pool
        if (m_CommandPool) {
            m_CommandPool->Shutdown(device);
            m_CommandPool.reset();
        }

        // Destroy framebuffers
        for (auto fb : m_Framebuffers) {
            if (fb != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(logicalDevice, fb, nullptr);
            }
        }
        m_Framebuffers.clear();

        // Destroy render pass (RHI abstraction)
        if (m_RenderPass) {
            m_RenderPass->Shutdown(device);
            m_RenderPass.reset();
        }

        // Destroy image views
        for (auto iv : m_ImageViews) {
            if (iv != VK_NULL_HANDLE) {
                vkDestroyImageView(logicalDevice, iv, nullptr);
            }
        }
        m_ImageViews.clear();

        // Destroy swapchain
        if (m_SwapChain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(logicalDevice, m_SwapChain, nullptr);
            m_SwapChain = VK_NULL_HANDLE;
        }

        m_Images.clear();
        m_CommandBuffers.clear();
    }

    bool VulkanSwapChain::AcquireNextImage(RHIDevice& device) {
        auto& vkDevice = static_cast<VulkanDevice&>(device);
        VkDevice logicalDevice = vkDevice.GetVkDevice();

        if (m_Fences.empty() || !m_Fences[m_CurrentFrame]) {
            VYRA_LOG_ERROR("[VulkanSwapChain] Fences not initialized");
            return false;
        }

        m_Fences[m_CurrentFrame]->Wait(device, UINT64_MAX);

        VkResult result = vkAcquireNextImageKHR(logicalDevice, m_SwapChain, UINT64_MAX,
            static_cast<VkSemaphore>(m_ImageAvailableSemaphores[m_CurrentFrame]->GetNativeSemaphore()), 
            VK_NULL_HANDLE, &m_ImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            return false; // Signal recreation needed
        }
        return true;
    }

    void VulkanSwapChain::BeginFrame(RHIDevice& device) {
        auto& vkDevice = static_cast<VulkanDevice&>(device);
        VkDevice logicalDevice = vkDevice.GetVkDevice();

        if (m_Fences.empty() || !m_Fences[m_CurrentFrame]) {
            VYRA_LOG_ERROR("[VulkanSwapChain] Fences not initialized");
            return;
        }

        m_Fences[m_CurrentFrame]->Reset(device);

        VkCommandBuffer cmd = m_CommandBuffers[m_CurrentFrame];
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(cmd, &beginInfo);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = static_cast<VkRenderPass>(m_RenderPass->GetNativeRenderPass());
        renderPassInfo.framebuffer = m_Framebuffers[m_ImageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_Extent;

        // Obsidian background color: dark graphite (#0D0E12)
        VkClearValue clearColor = { {{ 0.051f, 0.055f, 0.071f, 1.0f }} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_Extent.width);
        viewport.height = static_cast<float>(m_Extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_Extent;
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void VulkanSwapChain::EndFrame(RHIDevice& device) {
        VkCommandBuffer cmd = m_CommandBuffers[m_CurrentFrame];
        vkCmdEndRenderPass(cmd);
        vkEndCommandBuffer(cmd);
    }

    bool VulkanSwapChain::Present(RHIDevice& device) {
        auto& vkDevice = static_cast<VulkanDevice&>(device);

        VkCommandBuffer cmd = m_CommandBuffers[m_CurrentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { 
            static_cast<VkSemaphore>(m_ImageAvailableSemaphores[m_CurrentFrame]->GetNativeSemaphore()) 
        };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        VkSemaphore signalSemaphores[] = { 
            static_cast<VkSemaphore>(m_RenderFinishedSemaphores[m_CurrentFrame]->GetNativeSemaphore()) 
        };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkQueueSubmit(vkDevice.GetVkGraphicsQueue(), 1, &submitInfo, 
                     static_cast<VkFence>(m_Fences[m_CurrentFrame]->GetNativeFence()));

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_SwapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &m_ImageIndex;

        VkResult result = vkQueuePresentKHR(vkDevice.GetVkPresentQueue(), &presentInfo);
        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR;
    }

    VkSurfaceFormatKHR VulkanSwapChain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
        for (const auto& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }
        return formats[0];
    }

    VkPresentModeKHR VulkanSwapChain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& modes, bool vsync) {
        if (!vsync) {
            for (const auto& mode : modes) {
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR; // Guaranteed available, VSync
    }

    VkExtent2D VulkanSwapChain::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) {
        if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
            return capabilities.currentExtent;
        }
        VkExtent2D actual = { width, height };
        actual.width = (std::max)(capabilities.minImageExtent.width, (std::min)(capabilities.maxImageExtent.width, actual.width));
        actual.height = (std::max)(capabilities.minImageExtent.height, (std::min)(capabilities.maxImageExtent.height, actual.height));
        return actual;
    }

    void VulkanSwapChain::CreateImageViews(VkDevice device) {
        m_ImageViews.resize(m_Images.size());
        for (size_t i = 0; i < m_Images.size(); ++i) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_Images[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_ImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;
            vkCreateImageView(device, &viewInfo, nullptr, &m_ImageViews[i]);
        }
    }

    void VulkanSwapChain::CreateFramebuffers(VkDevice device) {
        m_Framebuffers.resize(m_ImageViews.size());
        for (size_t i = 0; i < m_ImageViews.size(); ++i) {
            VkImageView attachments[] = { m_ImageViews[i] };

            VkFramebufferCreateInfo fbInfo{};
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.renderPass = static_cast<VkRenderPass>(m_RenderPass->GetNativeRenderPass());
            fbInfo.attachmentCount = 1;
            fbInfo.pAttachments = attachments;
            fbInfo.width = m_Extent.width;
            fbInfo.height = m_Extent.height;
            fbInfo.layers = 1;

            VkResult result = vkCreateFramebuffer(device, &fbInfo, nullptr, &m_Framebuffers[i]);
            if (result != VK_SUCCESS) {
                VYRA_LOG_ERROR("[VulkanSwapChain] Failed to create framebuffer {}", i);
                m_Framebuffers[i] = VK_NULL_HANDLE;
            }
        }
    }

    void VulkanSwapChain::CreateCommandBuffers(VkDevice device) {
        m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        // Create command pool for swapchain
        m_CommandPool = RHICommandPool::Create();
        if (!m_CommandPool) {
            VYRA_LOG_ERROR("[VulkanSwapChain] Failed to create command pool");
            return;
        }

        if (!m_CommandPool->Init(device, m_GraphicsQueueFamilyIndex)) { 
            VYRA_LOG_ERROR("[VulkanSwapChain] Failed to initialize command pool");
            return;
        }

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = static_cast<VkCommandPool>(m_CommandPool->GetNativeCommandPool());
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

        VkResult result = vkAllocateCommandBuffers(device, &allocInfo, m_CommandBuffers.data());
        if (result != VK_SUCCESS) {
            VYRA_LOG_ERROR("[VulkanSwapChain] Failed to allocate command buffers");
            m_CommandBuffers.clear();
        }
    }

    void VulkanSwapChain::CreateSyncObjects(RHIDevice& device) {
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_Fences.resize(MAX_FRAMES_IN_FLIGHT);

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            m_ImageAvailableSemaphores[i] = RHISemaphore::Create();
            m_RenderFinishedSemaphores[i] = RHISemaphore::Create();
            m_Fences[i] = RHIFence::Create();

            if (!m_ImageAvailableSemaphores[i] || !m_ImageAvailableSemaphores[i]->Init(device)) {
                VYRA_LOG_ERROR("[VulkanSwapChain] Failed to create image available semaphore {}", i);
            }
            if (!m_RenderFinishedSemaphores[i] || !m_RenderFinishedSemaphores[i]->Init(device)) {
                VYRA_LOG_ERROR("[VulkanSwapChain] Failed to create render finished semaphore {}", i);
            }
            if (!m_Fences[i] || !m_Fences[i]->Init(device, true)) { // Signaled state
                VYRA_LOG_ERROR("[VulkanSwapChain] Failed to create fence {}", i);
            }
        }
    }

} // namespace vyra::rhi