#define VK_NO_PROTOTYPES
#include "vulkan_sync.hpp"
#include "vulkan_device.hpp"
#include "vyra/core/log.hpp"

namespace vyra::rhi {

    // -----------------------------------------------------------------------
    // VulkanRenderPass
    // -----------------------------------------------------------------------
    Scope<RHIRenderPass> RHIRenderPass::Create() {
        return CreateScope<VulkanRenderPass>();
    }

    bool VulkanRenderPass::Init(RHIDevice& device, const RenderPassCreateInfo& info) {
        VkDevice vkDevice = static_cast<VulkanDevice&>(device).GetVkDevice();

        // Convert attachment descriptions to Vulkan structures
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = static_cast<VkFormat>(info.ColorAttachment.Format);
        colorAttachment.samples = static_cast<VkSampleCountFlagBits>(info.ColorAttachment.Samples);
        colorAttachment.loadOp = static_cast<VkAttachmentLoadOp>(info.ColorAttachment.LoadOp);
        colorAttachment.storeOp = static_cast<VkAttachmentStoreOp>(info.ColorAttachment.StoreOp);
        colorAttachment.stencilLoadOp = static_cast<VkAttachmentLoadOp>(info.ColorAttachment.StencilLoadOp);
        colorAttachment.stencilStoreOp = static_cast<VkAttachmentStoreOp>(info.ColorAttachment.StencilStoreOp);
        colorAttachment.initialLayout = static_cast<VkImageLayout>(info.ColorAttachment.InitialLayout);
        colorAttachment.finalLayout = static_cast<VkImageLayout>(info.ColorAttachment.FinalLayout);

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = static_cast<VkFormat>(info.DepthAttachment.Format);
        depthAttachment.samples = static_cast<VkSampleCountFlagBits>(info.DepthAttachment.Samples);
        depthAttachment.loadOp = static_cast<VkAttachmentLoadOp>(info.DepthAttachment.LoadOp);
        depthAttachment.storeOp = static_cast<VkAttachmentStoreOp>(info.DepthAttachment.StoreOp);
        depthAttachment.stencilLoadOp = static_cast<VkAttachmentLoadOp>(info.DepthAttachment.StencilLoadOp);
        depthAttachment.stencilStoreOp = static_cast<VkAttachmentStoreOp>(info.DepthAttachment.StencilStoreOp);
        depthAttachment.initialLayout = static_cast<VkImageLayout>(info.DepthAttachment.InitialLayout);
        depthAttachment.finalLayout = static_cast<VkImageLayout>(info.DepthAttachment.FinalLayout);

        // Setup attachment references
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Setup subpass
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        // Setup render pass
        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        VkResult result = vkCreateRenderPass(vkDevice, &renderPassInfo, nullptr, &m_RenderPass);
        if (result != VK_SUCCESS) {
            VYRA_LOG_ERROR("[VulkanRenderPass] Failed to create render pass (VkResult: {})", static_cast<int>(result));
            return false;
        }

        VYRA_LOG_INFO("[VulkanRenderPass] Render pass created successfully.");
        return true;
    }

    void VulkanRenderPass::Shutdown(RHIDevice& device) {
        VkDevice vkDevice = static_cast<VulkanDevice&>(device).GetVkDevice();
        if (m_RenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(vkDevice, m_RenderPass, nullptr);
            m_RenderPass = VK_NULL_HANDLE;
        }
    }

    // -----------------------------------------------------------------------
    // VulkanCommandPool
    // -----------------------------------------------------------------------
    Scope<RHICommandPool> RHICommandPool::Create() {
        return CreateScope<VulkanCommandPool>();
    }

    bool VulkanCommandPool::Init(RHIDevice& device, uint32_t queueFamilyIndex) {
        VkDevice vkDevice = static_cast<VulkanDevice&>(device).GetVkDevice();

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndex;

        VkResult result = vkCreateCommandPool(vkDevice, &poolInfo, nullptr, &m_CommandPool);
        if (result != VK_SUCCESS) {
            VYRA_LOG_ERROR("[VulkanCommandPool] Failed to create command pool (VkResult: {})", static_cast<int>(result));
            return false;
        }

        VYRA_LOG_INFO("[VulkanCommandPool] Command pool created successfully.");
        return true;
    }

    void VulkanCommandPool::Shutdown(RHIDevice& device) {
        VkDevice vkDevice = static_cast<VulkanDevice&>(device).GetVkDevice();
        if (m_CommandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(vkDevice, m_CommandPool, nullptr);
            m_CommandPool = VK_NULL_HANDLE;
        }
    }

    // -----------------------------------------------------------------------
    // VulkanCommandBuffer
    // -----------------------------------------------------------------------
    Scope<RHICommandBuffer> RHICommandBuffer::Create() {
        return CreateScope<VulkanCommandBuffer>();
    }

    bool VulkanCommandBuffer::Begin() {
        if (m_CommandBuffer == VK_NULL_HANDLE) {
            VYRA_LOG_ERROR("[VulkanCommandBuffer] Cannot begin null command buffer");
            return false;
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VkResult result = vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);
        if (result != VK_SUCCESS) {
            VYRA_LOG_ERROR("[VulkanCommandBuffer] Failed to begin command buffer (VkResult: {})", static_cast<int>(result));
            return false;
        }

        return true;
    }

    void VulkanCommandBuffer::End() {
        if (m_CommandBuffer != VK_NULL_HANDLE) {
            VkResult result = vkEndCommandBuffer(m_CommandBuffer);
            if (result != VK_SUCCESS) {
                VYRA_LOG_ERROR("[VulkanCommandBuffer] Failed to end command buffer (VkResult: {})", static_cast<int>(result));
            }
        }
    }

    void VulkanCommandBuffer::Reset() {
        if (m_CommandBuffer != VK_NULL_HANDLE) {
            vkResetCommandBuffer(m_CommandBuffer, 0);
        }
    }

    // -----------------------------------------------------------------------
    // VulkanFence
    // -----------------------------------------------------------------------
    Scope<RHIFence> RHIFence::Create() {
        return CreateScope<VulkanFence>();
    }

    bool VulkanFence::Init(RHIDevice& device, bool signaled) {
        VkDevice vkDevice = static_cast<VulkanDevice&>(device).GetVkDevice();

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

        VkResult result = vkCreateFence(vkDevice, &fenceInfo, nullptr, &m_Fence);
        if (result != VK_SUCCESS) {
            VYRA_LOG_ERROR("[VulkanFence] Failed to create fence (VkResult: {})", static_cast<int>(result));
            return false;
        }

        return true;
    }

    void VulkanFence::Shutdown(RHIDevice& device) {
        VkDevice vkDevice = static_cast<VulkanDevice&>(device).GetVkDevice();
        if (m_Fence != VK_NULL_HANDLE) {
            vkDestroyFence(vkDevice, m_Fence, nullptr);
            m_Fence = VK_NULL_HANDLE;
        }
    }

    bool VulkanFence::Wait(RHIDevice& device, uint64_t timeout) {
        if (m_Fence == VK_NULL_HANDLE) {
            return false;
        }

        VkDevice vkDevice = static_cast<VulkanDevice&>(device).GetVkDevice();
        VkResult result = vkWaitForFences(vkDevice, 1, &m_Fence, VK_TRUE, timeout);
        return result == VK_SUCCESS;
    }

    void VulkanFence::Reset(RHIDevice& device) {
        if (m_Fence != VK_NULL_HANDLE) {
            VkDevice vkDevice = static_cast<VulkanDevice&>(device).GetVkDevice();
            vkResetFences(vkDevice, 1, &m_Fence);
        }
    }

    // -----------------------------------------------------------------------
    // VulkanSemaphore
    // -----------------------------------------------------------------------
    Scope<RHISemaphore> RHISemaphore::Create() {
        return CreateScope<VulkanSemaphore>();
    }

    bool VulkanSemaphore::Init(RHIDevice& device) {
        VkDevice vkDevice = static_cast<VulkanDevice&>(device).GetVkDevice();

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkResult result = vkCreateSemaphore(vkDevice, &semaphoreInfo, nullptr, &m_Semaphore);
        if (result != VK_SUCCESS) {
            VYRA_LOG_ERROR("[VulkanSemaphore] Failed to create semaphore (VkResult: {})", static_cast<int>(result));
            return false;
        }

        return true;
    }

    void VulkanSemaphore::Shutdown(RHIDevice& device) {
        VkDevice vkDevice = static_cast<VulkanDevice&>(device).GetVkDevice();
        if (m_Semaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(vkDevice, m_Semaphore, nullptr);
            m_Semaphore = VK_NULL_HANDLE;
        }
    }

} // namespace vyra::rhi