#pragma once

#define VK_NO_PROTOTYPES
#include <volk.h>
#include "vyra/rhi/rhi_resources.hpp"
#include <vector>

namespace vyra::rhi {

    // Finds a Vulkan memory type satisfying the requirements
    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // -----------------------------------------------------------------------
    // VulkanBuffer
    // -----------------------------------------------------------------------
    class VulkanBuffer : public RHIBuffer {
    public:
        VulkanBuffer() = default;
        ~VulkanBuffer() override = default;

        bool  Init(void* device, const BufferCreateInfo& info) override;
        void  Shutdown(void* device) override;
        void  Upload(void* device, const void* data, uint64_t size, uint64_t offset = 0) override;

        void* GetNativeBuffer() const override { return (void*)m_Buffer; }
        uint64_t GetSize() const override { return m_Size; }

        VkBuffer    GetVkBuffer() const { return m_Buffer; }
        VkDeviceMemory GetMemory() const { return m_Memory; }
        VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }

        void SetPhysicalDevice(VkPhysicalDevice physicalDevice) { m_PhysicalDevice = physicalDevice; }

    private:
        VkBuffer       m_Buffer{ VK_NULL_HANDLE };
        VkDeviceMemory m_Memory{ VK_NULL_HANDLE };
        uint64_t       m_Size{ 0 };
        VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
    };

    // -----------------------------------------------------------------------
    // VulkanShader
    // -----------------------------------------------------------------------
    class VulkanShader : public RHIShader {
    public:
        VulkanShader() = default;
        ~VulkanShader() override = default;

        bool Init(void* device, const ShaderCreateInfo& info) override;
        void Shutdown(void* device) override;

        void* GetNativeModule() const override { return (void*)m_ShaderModule; }
        ShaderStage GetStage() const override { return m_Stage; }

        VkShaderModule GetVkShaderModule() const { return m_ShaderModule; }

    private:
        VkShaderModule m_ShaderModule{ VK_NULL_HANDLE };
        ShaderStage    m_Stage{ ShaderStage::Vertex };
    };

    // -----------------------------------------------------------------------
    // VulkanPipeline
    // -----------------------------------------------------------------------
    class VulkanPipeline : public RHIPipeline {
    public:
        VulkanPipeline() = default;
        ~VulkanPipeline() override = default;

        bool Init(void* device, const PipelineCreateInfo& info) override;
        void Shutdown(void* device) override;

        void Bind(void* commandBuffer) override;

        void* GetNativePipeline() const override { return (void*)m_Pipeline; }
        void* GetNativeLayout() const override { return (void*)m_PipelineLayout; }

        VkPipeline GetVkPipeline() const { return m_Pipeline; }
        VkPipelineLayout GetVkPipelineLayout() const { return m_PipelineLayout; }

    private:
        VkPipeline       m_Pipeline{ VK_NULL_HANDLE };
        VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };
    };

} // namespace vyra::rhi
