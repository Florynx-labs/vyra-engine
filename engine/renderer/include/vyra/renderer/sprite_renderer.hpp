#pragma once

#include "vyra/core/base.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace vyra::renderer {

    // 2D Vertex for sprite rendering
    struct Vertex2D {
        glm::vec2 Position;
        glm::vec2 TexCoord;
        glm::vec4 Color;
    };

    // Sprite quad for rendering
    struct SpriteQuad {
        Vertex2D Vertices[4];
        uint32_t Indices[6];
    };

    // Per-sprite push constants
    struct SpritePushConstants {
        glm::mat4 ViewProjection{ 1.0f };
        glm::mat4 Transform{ 1.0f };
        glm::vec4 Color{ 1.0f };
        float TilingFactor{ 1.0f };
        glm::vec2 TexCoordsMin{ 0.0f, 0.0f };
        glm::vec2 TexCoordsMax{ 1.0f, 1.0f };
    };

    // GPU resources for sprite batch
    struct SpriteBatch {
        void* VertexBuffer{ nullptr };
        void* IndexBuffer{ nullptr };
        uint32_t VertexCount{ 0 };
        uint32_t IndexCount{ 0 };
    };

    class VYRA_API SpriteRenderer {
    public:
        SpriteRenderer() = default;
        ~SpriteRenderer() = default;

        /// Initialize sprite renderer with device and render pass
        bool Init(void* device, void* physicalDevice, void* renderPass);
        void Shutdown();

        /// Begin sprite batch
        void BeginScene(const glm::mat4& viewProjection);
        
        /// End sprite batch
        void EndScene();

        /// Draw a sprite quad
        void DrawSprite(void* commandBuffer, const glm::mat4& transform, 
                       const glm::vec4& color = glm::vec4(1.0f),
                       const glm::vec4& texCoords = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
                       float tilingFactor = 1.0f);

        /// Create default quad geometry
        static SpriteQuad CreateDefaultQuad();

    private:
        void* m_Device{ nullptr };
        void* m_PhysicalDevice{ nullptr };
        void* m_RenderPass{ nullptr };
        glm::mat4 m_ViewProjection{ 1.0f };
    };

} // namespace vyra::renderer