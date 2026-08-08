#pragma once

#include "vyra/rhi/rhi.hpp"
#include "vulkan_context.hpp"

#include <vector>
#include <set>

namespace vyra::rhi {

    class VulkanDevice : public RHIDevice {
    public:
        VulkanDevice() = default;
        ~VulkanDevice() override;

        bool Init(RHIContext& context, void* surface) override;
        void Shutdown() override;

        void* GetNativeDevice() const override { return (void*)m_Device; }
        void* GetPhysicalDevice() const override { return (void*)m_PhysicalDevice; }
        void* GetGraphicsQueue() const override { return (void*)m_GraphicsQueue; }
        void* GetPresentQueue() const override { return (void*)m_PresentQueue; }
        QueueFamilyIndices GetQueueFamilies() const override { return m_QueueFamilyIndices; }

        void WaitIdle() override;

        VkDevice GetVkDevice() const { return m_Device; }
        VkPhysicalDevice GetVkPhysicalDevice() const { return m_PhysicalDevice; }
        VkQueue GetVkGraphicsQueue() const { return m_GraphicsQueue; }
        VkQueue GetVkPresentQueue() const { return m_PresentQueue; }

    private:
        VkPhysicalDevice PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkDevice m_Device = VK_NULL_HANDLE;
        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;
        QueueFamilyIndices m_QueueFamilyIndices;

        const std::vector<const char*> m_DeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
    };

} // namespace vyra::rhi
