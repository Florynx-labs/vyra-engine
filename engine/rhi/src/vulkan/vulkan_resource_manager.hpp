#pragma once

#define VK_NO_PROTOTYPES
#include <volk.h>
#include "vyra/rhi/rhi.hpp"
#include <memory>
#include <vector>
#include <functional>

namespace vyra::rhi {

    // Resource handle with automatic cleanup
    class VulkanResourceHandle {
    public:
        using CleanupFunc = std::function<void(VkDevice)>;

        VulkanResourceHandle() = default;
        VulkanResourceHandle(CleanupFunc cleanup) : m_Cleanup(cleanup) {}
        ~VulkanResourceHandle() {
            if (m_Cleanup && m_Device) {
                m_Cleanup(m_Device);
            }
        }

        // Move-only
        VulkanResourceHandle(const VulkanResourceHandle&) = delete;
        VulkanResourceHandle& operator=(const VulkanResourceHandle&) = delete;
        VulkanResourceHandle(VulkanResourceHandle&& other) noexcept 
            : m_Device(other.m_Device), m_Cleanup(std::move(other.m_Cleanup)) {
            other.m_Device = VK_NULL_HANDLE;
            other.m_Cleanup = nullptr;
        }
        VulkanResourceHandle& operator=(VulkanResourceHandle&& other) noexcept {
            if (this != &other) {
                if (m_Cleanup && m_Device) {
                    m_Cleanup(m_Device);
                }
                m_Device = other.m_Device;
                m_Cleanup = std::move(other.m_Cleanup);
                other.m_Device = VK_NULL_HANDLE;
                other.m_Cleanup = nullptr;
            }
            return *this;
        }

        void SetDevice(VkDevice device) { m_Device = device; }
        VkDevice GetDevice() const { return m_Device; }

    private:
        VkDevice m_Device{ VK_NULL_HANDLE };
        CleanupFunc m_Cleanup;
    };

    // RAII resource manager for Vulkan objects
    class VulkanResourceManager {
    public:
        VulkanResourceManager() = default;
        ~VulkanResourceManager() {
            Shutdown();
        }

        void SetDevice(VkDevice device) { m_Device = device; }
        VkDevice GetDevice() const { return m_Device; }

        // Register a resource with automatic cleanup
        template<typename T>
        void RegisterResource(T handle, std::function<void(VkDevice, T)> cleanup) {
            if (!m_Device) return;
            
            auto resource = std::make_unique<VulkanResourceHandle>(
                [cleanup, handle](VkDevice device) {
                    if (handle != VK_NULL_HANDLE) {
                        cleanup(device, handle);
                    }
                }
            );
            resource->SetDevice(m_Device);
            m_Resources.push_back(std::move(resource));
        }

        // Shutdown all resources in proper order
        void Shutdown() {
            // Destroy in reverse order (LIFO) for proper dependency cleanup
            for (auto it = m_Resources.rbegin(); it != m_Resources.rend(); ++it) {
                it->reset();
            }
            m_Resources.clear();
        }

        // Get resource count
        size_t GetResourceCount() const { return m_Resources.size(); }

    private:
        VkDevice m_Device{ VK_NULL_HANDLE };
        std::vector<std::unique_ptr<VulkanResourceHandle>> m_Resources;
    };

} // namespace vyra::rhi