#pragma once

#include "vyra/core/base.hpp"
#include "vyra/renderer/mesh.hpp"
#include "vyra/rhi/rhi.hpp"
#include "vyra/rhi/rhi_resources.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace vyra::renderer {

    struct MeshRenderCommand {
        const Mesh*  SourceMesh{ nullptr };
        glm::mat4    Transform{ 1.0f };
        glm::vec4    Color{ 1.0f };
    };

    // Per-object push constants passed to the vertex shader
    struct PushConstants {
        glm::mat4 ViewProjection{ 1.0f };
        glm::mat4 Model{ 1.0f };
        glm::vec4 Color{ 1.0f };
    };

    // Uploaded GPU buffers for a single mesh
    struct GPUMesh {
        Scope<rhi::RHIBuffer> VertexBuffer;
        Scope<rhi::RHIBuffer> IndexBuffer;
        uint32_t              IndexCount{ 0 };
    };

    class VYRA_API MeshRenderer {
    public:
        MeshRenderer() = default;
        ~MeshRenderer() = default;

        /// Must be called once with valid device+renderPass pointers.
        bool Init(void* device, void* physicalDevice, void* renderPass);
        void Shutdown();

        /// Upload geometry to the GPU (vertex + index buffers).
        Scope<GPUMesh> UploadMesh(void* device, const Mesh& mesh);

        /// Begin/End a render pass frame.
        void BeginScene(const glm::mat4& viewProjection);
        void EndScene();

        /// Submit a draw call (must be called between Begin/EndScene).
        void DrawMesh(void* commandBuffer, const GPUMesh& gpuMesh, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f));

    private:
        void* m_Device{ nullptr };
        void* m_PhysicalDevice{ nullptr };
        glm::mat4 m_ViewProjection{ 1.0f };
    };

} // namespace vyra::renderer
