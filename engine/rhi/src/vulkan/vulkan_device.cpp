#include "vulkan_device.hpp"
#include "vyra/core/log.hpp"

namespace vyra::rhi {

    Scope<RHIDevice> RHIDevice::Create() {
        return CreateScope<VulkanDevice>();
    }

    VulkanDevice::~VulkanDevice() {
        Shutdown();
    }

    bool VulkanDevice::Init(RHIContext& context, void* surface) {
        VkInstance instance = static_cast<VulkanContext&>(context).GetVkInstance();
        VkSurfaceKHR vkSurface = static_cast<VkSurfaceKHR>(surface);

        // Pick physical device
        m_PhysicalDevice = PickPhysicalDevice(instance, vkSurface);
        if (m_PhysicalDevice == VK_NULL_HANDLE) {
            VYRA_LOG_CHANNEL(LogChannel::Renderer, error, "Failed to find a suitable Vulkan GPU");
            return false;
        }

        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);
        VYRA_LOG_CHANNEL(LogChannel::Renderer, info, "Selected GPU: {}", props.deviceName);

        // Find queue families
        m_QueueFamilyIndices = FindQueueFamilies(m_PhysicalDevice, vkSurface);

        // Create logical device
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            m_QueueFamilyIndices.GraphicsFamily.value(),
            m_QueueFamilyIndices.PresentFamily.value()
        };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.fillModeNonSolid = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

        VkResult result = vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device);
        if (result != VK_SUCCESS) {
            VYRA_LOG_CHANNEL(LogChannel::Renderer, error, "Failed to create Vulkan logical device (VkResult: {})", static_cast<int>(result));
            return false;
        }

        volkLoadDevice(m_Device);

        vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.PresentFamily.value(), 0, &m_PresentQueue);

        VYRA_LOG_CHANNEL(LogChannel::Renderer, info, "Vulkan logical device created successfully.");
        return true;
    }

    void VulkanDevice::Shutdown() {
        if (m_Device != VK_NULL_HANDLE) {
            vkDestroyDevice(m_Device, nullptr);
            m_Device = VK_NULL_HANDLE;
        }
    }

    void VulkanDevice::WaitIdle() {
        if (m_Device != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(m_Device);
        }
    }

    VkPhysicalDevice VulkanDevice::PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) return VK_NULL_HANDLE;

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // Prefer discrete GPU
        VkPhysicalDevice fallback = VK_NULL_HANDLE;
        for (const auto& device : devices) {
            QueueFamilyIndices indices = FindQueueFamilies(device, surface);
            if (!indices.IsComplete()) continue;
            if (!CheckDeviceExtensionSupport(device)) continue;

            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);

            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                return device;
            }
            if (fallback == VK_NULL_HANDLE) {
                fallback = device;
            }
        }
        return fallback;
    }

    QueueFamilyIndices VulkanDevice::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.GraphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                indices.PresentFamily = i;
            }

            if (indices.IsComplete()) break;
        }
        return indices;
    }

    bool VulkanDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());
        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }
        return requiredExtensions.empty();
    }

} // namespace vyra::rhi
