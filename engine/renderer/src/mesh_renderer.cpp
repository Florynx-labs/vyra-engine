#include "vyra/renderer/mesh_renderer.hpp"
#include "vyra/core/log.hpp"

#define VK_NO_PROTOTYPES
#include <volk.h>

namespace vyra::renderer {

    bool MeshRenderer::Init(void* device, void* physicalDevice, void* renderPass) {
        m_Device = device;
        m_PhysicalDevice = physicalDevice;
        VYRA_LOG_INFO("[MeshRenderer] Initialized.");
        return true;
    }

    void MeshRenderer::Shutdown() {
        VYRA_LOG_INFO("[MeshRenderer] Shut down.");
    }

    Scope<GPUMesh> MeshRenderer::UploadMesh(void* device, const Mesh& mesh) {
        auto gpuMesh = CreateScope<GPUMesh>();
        gpuMesh->IndexCount = static_cast<uint32_t>(mesh.Indices.size());

        // Upload vertex data via host-visible buffer
        uint64_t vertexSize = mesh.Vertices.size() * sizeof(Vertex3D);
        if (vertexSize > 0) {
            gpuMesh->VertexBuffer = rhi::RHIBuffer::CreateVertex(device, m_PhysicalDevice, mesh.Vertices.data(), vertexSize);
        }

        // Upload index data
        uint64_t indexSize = mesh.Indices.size() * sizeof(uint32_t);
        if (indexSize > 0) {
            gpuMesh->IndexBuffer = rhi::RHIBuffer::CreateIndex(device, m_PhysicalDevice, mesh.Indices.data(), indexSize);
        }

        VYRA_LOG_INFO("[MeshRenderer] Uploaded mesh '{0}': {1} vertices, {2} indices",
            mesh.Name, mesh.Vertices.size(), mesh.Indices.size());

        return gpuMesh;
    }

    void MeshRenderer::BeginScene(const glm::mat4& viewProjection) {
        m_ViewProjection = viewProjection;
    }

    void MeshRenderer::EndScene() {
        // Reserved for future batching / flush
    }

    void MeshRenderer::DrawMesh(void* rawCommandBuffer, const GPUMesh& gpuMesh,
                                 const glm::mat4& transform, const glm::vec4& color) {
        VkCommandBuffer cmd = static_cast<VkCommandBuffer>(rawCommandBuffer);

        if (!gpuMesh.VertexBuffer || !gpuMesh.IndexBuffer) {
            VYRA_LOG_WARN("[MeshRenderer] DrawMesh called with null buffers — skipping.");
            return;
        }

        // Bind vertex buffer
        VkBuffer vertBuf = static_cast<VkBuffer>(gpuMesh.VertexBuffer->GetNativeBuffer());
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmd, 0, 1, &vertBuf, offsets);

        // Bind index buffer
        VkBuffer idxBuf = static_cast<VkBuffer>(gpuMesh.IndexBuffer->GetNativeBuffer());
        vkCmdBindIndexBuffer(cmd, idxBuf, 0, VK_INDEX_TYPE_UINT32);

        // Issue draw
        vkCmdDrawIndexed(cmd, gpuMesh.IndexCount, 1, 0, 0, 0);
    }

} // namespace vyra::renderer
