#include "vulkan_context.hpp"
#include "vyra/rhi/rhi_error.hpp"
#include "vyra/core/log.hpp"
#include "vyra/core/assert.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace vyra::rhi {

    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        switch (messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                VYRA_LOG_CHANNEL(LogChannel::Renderer, trace, "[Vulkan] {}", pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                VYRA_LOG_CHANNEL(LogChannel::Renderer, info, "[Vulkan] {}", pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                VYRA_LOG_CHANNEL(LogChannel::Renderer, warn, "[Vulkan] {}", pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                VYRA_LOG_CHANNEL(LogChannel::Renderer, error, "[Vulkan] {}", pCallbackData->pMessage);
                break;
            default:
                break;
        }
        return VK_FALSE;
    }

    Scope<RHIContext> RHIContext::Create() {
        return CreateScope<VulkanContext>();
    }

    VulkanContext::~VulkanContext() {
        Shutdown();
    }

    bool VulkanContext::Init(const ContextCreateInfo& info) {
        VYRA_LOG_CHANNEL(LogChannel::Renderer, info, "Initializing Vulkan RHI Context...");

        // Initialize Volk (dynamic Vulkan function loader)
        VkResult volkResult = volkInitialize();
        if (volkResult != VK_SUCCESS) {
            RHIResult result = RHIResult::FromVkResult(volkResult, "Failed to initialize Volk (Vulkan loader)");
            VYRA_LOG_CHANNEL(LogChannel::Renderer, error, "{}", result.GetMessage());
            return false;
        }

        m_ValidationEnabled = info.EnableValidation;

        if (m_ValidationEnabled && !CheckValidationLayerSupport()) {
            VYRA_LOG_CHANNEL(LogChannel::Renderer, warn, "Validation layers requested but not available. Disabling.");
            m_ValidationEnabled = false;
        }

        // Application Info
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = info.AppName.c_str();
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, info.AppVersion, 0);
        appInfo.pEngineName = "VYRA Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
        appInfo.apiVersion = VK_API_VERSION_1_4; // Updated to Vulkan 1.4

        // Extensions
        if (!SDL_WasInit(SDL_INIT_VIDEO)) {
            SDL_Init(SDL_INIT_VIDEO);
        }

        uint32_t sdlExtCount = 0;
        const char* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtCount);

        std::vector<const char*> extensions;
        if (sdlExtensions) {
            for (uint32_t i = 0; i < sdlExtCount; ++i) {
                extensions.push_back(sdlExtensions[i]);
            }
        }

        if (m_ValidationEnabled) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // Instance Create Info
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (m_ValidationEnabled) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

            debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = VulkanDebugCallback;
            createInfo.pNext = &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
        if (result != VK_SUCCESS) {
            RHIResult rhiResult = RHIResult::FromVkResult(result, "Failed to create Vulkan instance");
            VYRA_LOG_CHANNEL(LogChannel::Renderer, error, "{}", rhiResult.GetMessage());
            return false;
        }

        // Load instance-level Vulkan functions via Volk
        volkLoadInstance(m_Instance);

        if (m_ValidationEnabled) {
            SetupDebugMessenger();
        }

        VYRA_LOG_CHANNEL(LogChannel::Renderer, info, "Vulkan instance created successfully.");
        return true;
    }

    void VulkanContext::Shutdown() {
        if (m_DebugMessenger != VK_NULL_HANDLE) {
            vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
            m_DebugMessenger = VK_NULL_HANDLE;
        }

        if (m_Instance != VK_NULL_HANDLE) {
            vkDestroyInstance(m_Instance, nullptr);
            m_Instance = VK_NULL_HANDLE;
        }
    }

    bool VulkanContext::CheckValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : m_ValidationLayers) {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) return false;
        }
        return true;
    }

    void VulkanContext::SetupDebugMessenger() {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = VulkanDebugCallback;

        VkResult result = vkCreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
        if (result != VK_SUCCESS) {
            VYRA_LOG_CHANNEL(LogChannel::Renderer, warn, "Failed to set up Vulkan debug messenger");
        }
    }

} // namespace vyra::rhi
