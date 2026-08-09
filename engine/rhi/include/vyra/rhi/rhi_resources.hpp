#pragma once

#include "vyra/core/base.hpp"
#include <cstdint>
#include <vector>
#include <string>

namespace vyra::rhi {

    // Forward declaration
    class RHIRenderPass;

    enum class BufferUsage : uint32_t {
        Vertex   = 0,
        Index    = 1,
        Uniform  = 2,
        Staging  = 3,
    };

    enum class MemoryAccess : uint32_t {
        GPUOnly = 0,  // Device-local (fastest for draw)
        CPUToGPU = 1, // Host-visible + coherent (staging / UBO)
    };

    struct BufferCreateInfo {
        uint64_t     Size{ 0 };
        BufferUsage  Usage{ BufferUsage::Vertex };
        MemoryAccess Memory{ MemoryAccess::GPUOnly };
    };

    class VYRA_API RHIBuffer {
    public:
        virtual ~RHIBuffer() = default;

        virtual bool  Init(void* device, const BufferCreateInfo& info) = 0;
        virtual void  Shutdown(void* device) = 0;

        virtual void  Upload(void* device, const void* data, uint64_t size, uint64_t offset = 0) = 0;

        virtual void* GetNativeBuffer() const = 0;
        virtual uint64_t GetSize() const = 0;

        static Scope<RHIBuffer> CreateVertex(void* device, void* physicalDevice, const void* data, uint64_t size);
        static Scope<RHIBuffer> CreateIndex(void* device, void* physicalDevice, const void* data, uint64_t size);
    };

    // -----------------------------------------------------------------------
    // Shader abstraction (loads SPIR-V bytecode)
    // -----------------------------------------------------------------------
    enum class ShaderStage : uint32_t {
        Vertex   = 0,
        Fragment = 1,
    };

    struct ShaderCreateInfo {
        ShaderStage        Stage;
        std::vector<uint32_t> ByteCode; // SPIR-V words
        std::string        EntryPoint{ "main" };
    };

    class VYRA_API RHIShader {
    public:
        virtual ~RHIShader() = default;

        virtual bool Init(void* device, const ShaderCreateInfo& info) = 0;
        virtual void Shutdown(void* device) = 0;

        virtual void* GetNativeModule() const = 0;
        virtual ShaderStage GetStage() const = 0;

        static Scope<RHIShader> Create(void* device, ShaderStage stage, const std::vector<uint32_t>& spirv);
    };

    // -----------------------------------------------------------------------
    // Graphics Pipeline abstraction
    // -----------------------------------------------------------------------
    struct VertexAttribute {
        uint32_t    Location{ 0 };
        uint32_t    Binding{ 0 };
        uint32_t    Format{ 0 }; // VkFormat value (e.g. R32G32B32_SFLOAT = 106)
        uint32_t    Offset{ 0 };
    };

    struct PipelineCreateInfo {
        RHIShader*                   VertexShader{ nullptr };
        RHIShader*                   FragmentShader{ nullptr };
        std::vector<VertexAttribute> VertexAttributes;
        uint32_t                     VertexStride{ 0 };
        class RHIRenderPass*         RenderPass{ nullptr };
    };

    class VYRA_API RHIPipeline {
    public:
        virtual ~RHIPipeline() = default;

        virtual bool Init(void* device, const PipelineCreateInfo& info) = 0;
        virtual void Shutdown(void* device) = 0;

        virtual void Bind(void* commandBuffer) = 0;

        virtual void* GetNativePipeline() const = 0;
        virtual void* GetNativeLayout() const = 0;

        static Scope<RHIPipeline> Create(void* device, const PipelineCreateInfo& info);
    };

} // namespace vyra::rhi
