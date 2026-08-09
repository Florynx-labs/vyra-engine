#include "vyra/renderer/sprite_renderer.hpp"
#include "vyra/core/log.hpp"

#define VK_NO_PROTOTYPES
#include <volk.h>

namespace vyra::renderer {

    bool SpriteRenderer::Init(void* device, void* physicalDevice, void* renderPass) {
        m_Device = device;
        m_PhysicalDevice = physicalDevice;
        m_RenderPass = renderPass;
        
        VYRA_LOG_INFO("[SpriteRenderer] Initialized.");
        return true;
    }

    void SpriteRenderer::Shutdown() {
        VYRA_LOG_INFO("[SpriteRenderer] Shut down.");
    }

    void SpriteRenderer::BeginScene(const glm::mat4& viewProjection) {
        m_ViewProjection = viewProjection;
    }

    void SpriteRenderer::EndScene() {
        // Reserved for future batch flushing
    }

    void SpriteRenderer::DrawSprite(void* commandBuffer, const glm::mat4& transform,
                                    const glm::vec4& color, const glm::vec4& texCoords,
                                    float tilingFactor) {
        // TODO: Implement actual sprite rendering
        // This would:
        // 1. Set up push constants
        // 2. Bind sprite pipeline
        // 3. Bind vertex/index buffers
        // 4. Draw indexed
        (void)commandBuffer;
        (void)transform;
        (void)color;
        (void)texCoords;
        (void)tilingFactor;
    }

    SpriteQuad SpriteRenderer::CreateDefaultQuad() {
        SpriteQuad quad;
        
        // Create quad vertices (centered at origin)
        quad.Vertices[0] = { { -0.5f, -0.5f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
        quad.Vertices[1] = { { 0.5f, -0.5f }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
        quad.Vertices[2] = { { 0.5f, 0.5f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
        quad.Vertices[3] = { { -0.5f, 0.5f }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
        
        // Create triangle indices
        quad.Indices[0] = 0;
        quad.Indices[1] = 1;
        quad.Indices[2] = 2;
        quad.Indices[3] = 0;
        quad.Indices[4] = 2;
        quad.Indices[5] = 3;
        
        return quad;
    }

} // namespace vyra::renderer