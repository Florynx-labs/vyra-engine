#pragma once

#include "vyra/rhi/rhi.hpp"
#include "vyra/core/log.hpp"

#include <volk.h>

#include <vector>
#include <string>

namespace vyra::rhi {

    class VulkanContext : public RHIContext {
    public:
        VulkanContext() = default;
        ~VulkanContext() override;

        bool Init(const ContextCreateInfo& info) override;
        void Shutdown() override;

        void* GetInstance() const override { return (void*)m_Instance; }
        bool IsValidationEnabled() const override { return m_ValidationEnabled; }

        VkInstance GetVkInstance() const { return m_Instance; }

    private:
        bool CheckValidationLayerSupport();
        void SetupDebugMessenger();

        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        bool m_ValidationEnabled = false;

        const std::vector<const char*> m_ValidationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
    };

} // namespace vyra::rhi
