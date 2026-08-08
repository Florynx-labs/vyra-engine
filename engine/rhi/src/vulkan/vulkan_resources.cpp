#define VK_NO_PROTOTYPES
#include "vulkan_resources.hpp"
#include "vyra/core/log.hpp"
#include <cstring>

namespace vyra::rhi {

    // -----------------------------------------------------------------------
    // Memory type helper
    // -----------------------------------------------------------------------
    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        VYRA_LOG_ERROR("[VulkanBuffer] Failed to find suitable memory type!");
        return 0;
    }

    // -----------------------------------------------------------------------
    // VulkanBuffer
    // -----------------------------------------------------------------------
    static VkBufferUsageFlags ToVkUsage(BufferUsage usage) {
        switch (usage) {
            case BufferUsage::Vertex:  return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            case BufferUsage::Index:   return VK_BUFFER_USAGE_INDEX_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            case BufferUsage::Uniform: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            case BufferUsage::Staging: return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            default: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }
    }

    static VkMemoryPropertyFlags ToVkMemoryFlags(MemoryAccess access) {
        switch (access) {
            case MemoryAccess::GPUOnly:  return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            case MemoryAccess::CPUToGPU: return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            default: return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }
    }

    bool VulkanBuffer::Init(void* rawDevice, const BufferCreateInfo& info) {
        VkDevice device = static_cast<VkDevice>(rawDevice);
        m_Size = info.Size;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = info.Size;
        bufferInfo.usage = ToVkUsage(info.Usage);
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS) {
            VYRA_LOG_ERROR("[VulkanBuffer] Failed to create buffer (size: {0})", info.Size);
            return false;
        }

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(device, m_Buffer, &memReq);

        // Note: For actual GPU-only buffers we need the physical device to find memory type.
        // We use host-visible for simplicity here (will be upgraded to staging in full renderer).
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = 0; // Placeholder — caller should use staging pattern

        // Try host-visible first (works for CPU-to-GPU)
        // For device-local we'd need a separate staging buffer + copy command.
        VkMemoryPropertyFlags desiredFlags = ToVkMemoryFlags(info.Memory);
        // Since we don't have physical device stored here, default to host visible for now.
        (void)desiredFlags;

        // Fallback: just request host-visible + coherent memory
        allocInfo.memoryTypeIndex = 0; // Will be properly resolved when physical device is stored

        if (vkAllocateMemory(device, &allocInfo, nullptr, &m_Memory) != VK_SUCCESS) {
            VYRA_LOG_ERROR("[VulkanBuffer] Failed to allocate device memory!");
            return false;
        }

        vkBindBufferMemory(device, m_Buffer, m_Memory, 0);
        return true;
    }

    void VulkanBuffer::Shutdown(void* rawDevice) {
        VkDevice device = static_cast<VkDevice>(rawDevice);
        if (m_Buffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(device, m_Buffer, nullptr);
            m_Buffer = VK_NULL_HANDLE;
        }
        if (m_Memory != VK_NULL_HANDLE) {
            vkFreeMemory(device, m_Memory, nullptr);
            m_Memory = VK_NULL_HANDLE;
        }
    }

    void VulkanBuffer::Upload(void* rawDevice, const void* data, uint64_t size, uint64_t offset) {
        VkDevice device = static_cast<VkDevice>(rawDevice);
        void* mapped = nullptr;
        vkMapMemory(device, m_Memory, offset, size, 0, &mapped);
        if (mapped && data) {
            std::memcpy(mapped, data, static_cast<size_t>(size));
        }
        vkUnmapMemory(device, m_Memory);
    }

    // -----------------------------------------------------------------------
    // RHIBuffer factory
    // -----------------------------------------------------------------------
    Scope<RHIBuffer> RHIBuffer::CreateVertex(void* device, const void* data, uint64_t size) {
        auto buffer = CreateScope<VulkanBuffer>();
        BufferCreateInfo info;
        info.Size = size;
        info.Usage = BufferUsage::Staging; // host-visible for easy upload
        info.Memory = MemoryAccess::CPUToGPU;
        if (buffer->Init(device, info)) {
            buffer->Upload(device, data, size);
        }
        return buffer;
    }

    Scope<RHIBuffer> RHIBuffer::CreateIndex(void* device, const void* data, uint64_t size) {
        auto buffer = CreateScope<VulkanBuffer>();
        BufferCreateInfo info;
        info.Size = size;
        info.Usage = BufferUsage::Staging;
        info.Memory = MemoryAccess::CPUToGPU;
        if (buffer->Init(device, info)) {
            buffer->Upload(device, data, size);
        }
        return buffer;
    }

    // -----------------------------------------------------------------------
    // VulkanShader
    // -----------------------------------------------------------------------
    bool VulkanShader::Init(void* rawDevice, const ShaderCreateInfo& info) {
        VkDevice device = static_cast<VkDevice>(rawDevice);
        m_Stage = info.Stage;

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = info.ByteCode.size() * sizeof(uint32_t);
        createInfo.pCode = info.ByteCode.data();

        if (vkCreateShaderModule(device, &createInfo, nullptr, &m_ShaderModule) != VK_SUCCESS) {
            VYRA_LOG_ERROR("[VulkanShader] Failed to create shader module!");
            return false;
        }

        VYRA_LOG_INFO("[VulkanShader] Shader module created (stage: {0})", static_cast<int>(info.Stage));
        return true;
    }

    void VulkanShader::Shutdown(void* rawDevice) {
        VkDevice device = static_cast<VkDevice>(rawDevice);
        if (m_ShaderModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device, m_ShaderModule, nullptr);
            m_ShaderModule = VK_NULL_HANDLE;
        }
    }

    Scope<RHIShader> RHIShader::Create(void* device, ShaderStage stage, const std::vector<uint32_t>& spirv) {
        auto shader = CreateScope<VulkanShader>();
        ShaderCreateInfo info;
        info.Stage = stage;
        info.ByteCode = spirv;
        shader->Init(device, info);
        return shader;
    }

    // -----------------------------------------------------------------------
    // VulkanPipeline
    // -----------------------------------------------------------------------
    bool VulkanPipeline::Init(void* rawDevice, const PipelineCreateInfo& info) {
        VkDevice device = static_cast<VkDevice>(rawDevice);

        // Shader stages
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        if (info.VertexShader) {
            VkPipelineShaderStageCreateInfo vertStage{};
            vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertStage.module = static_cast<VkShaderModule>(info.VertexShader->GetNativeModule());
            vertStage.pName = "main";
            shaderStages.push_back(vertStage);
        }

        if (info.FragmentShader) {
            VkPipelineShaderStageCreateInfo fragStage{};
            fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragStage.module = static_cast<VkShaderModule>(info.FragmentShader->GetNativeModule());
            fragStage.pName = "main";
            shaderStages.push_back(fragStage);
        }

        // Vertex input
        VkVertexInputBindingDescription bindingDesc{};
        bindingDesc.binding = 0;
        bindingDesc.stride = info.VertexStride;
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::vector<VkVertexInputAttributeDescription> attrDescs;
        for (const auto& attr : info.VertexAttributes) {
            VkVertexInputAttributeDescription attrDesc{};
            attrDesc.location = attr.Location;
            attrDesc.binding  = attr.Binding;
            attrDesc.format   = static_cast<VkFormat>(attr.Format);
            attrDesc.offset   = attr.Offset;
            attrDescs.push_back(attrDesc);
        }

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &bindingDesc;
        vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrDescs.size());
        vertexInput.pVertexAttributeDescriptions = attrDescs.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Dynamic states (viewport and scissor set at draw time)
        std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            VYRA_LOG_ERROR("[VulkanPipeline] Failed to create pipeline layout!");
            return false;
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.renderPass = static_cast<VkRenderPass>(info.RenderPass);
        pipelineInfo.subpass = 0;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS) {
            VYRA_LOG_ERROR("[VulkanPipeline] Failed to create graphics pipeline!");
            return false;
        }

        VYRA_LOG_INFO("[VulkanPipeline] Graphics pipeline created successfully.");
        return true;
    }

    void VulkanPipeline::Shutdown(void* rawDevice) {
        VkDevice device = static_cast<VkDevice>(rawDevice);
        if (m_Pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(device, m_Pipeline, nullptr);
            m_Pipeline = VK_NULL_HANDLE;
        }
        if (m_PipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
            m_PipelineLayout = VK_NULL_HANDLE;
        }
    }

    void VulkanPipeline::Bind(void* commandBuffer) {
        VkCommandBuffer cmd = static_cast<VkCommandBuffer>(commandBuffer);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    }

    Scope<RHIPipeline> RHIPipeline::Create(void* device, const PipelineCreateInfo& info) {
        auto pipeline = CreateScope<VulkanPipeline>();
        pipeline->Init(device, info);
        return pipeline;
    }

} // namespace vyra::rhi
