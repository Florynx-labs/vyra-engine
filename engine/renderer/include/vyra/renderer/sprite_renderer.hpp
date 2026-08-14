#pragma once

#include "vyra/core/base.hpp"
#include "vyra/core/log.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <chrono>
#include <string>

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

    // Renderer state for validation
    enum class RendererState {
        Uninitialized,
        Ready,
        SceneActive,
        Error
    };

    // Renderer statistics for monitoring
    struct RendererStatistics {
        uint32_t DrawCalls{ 0 };
        uint32_t SpritesDrawn{ 0 };
        uint32_t VerticesProcessed{ 0 };
        double FrameTimeMs{ 0.0 };
        double AverageFrameTimeMs{ 0.0 };
        uint32_t FrameCount{ 0 };
    };

    class VYRA_API SpriteRenderer {
    public:
        SpriteRenderer() = default;
        ~SpriteRenderer() = default;

        /// Initialize sprite renderer with device and render pass
        bool Init(void* device, void* physicalDevice, void* renderPass);
        void Shutdown();

        /// Begin sprite batch
        bool BeginScene(const glm::mat4& viewProjection);
        
        /// End sprite batch
        bool EndScene();

        /// Draw a sprite quad
        bool DrawSprite(void* commandBuffer, const glm::mat4& transform, 
                       const glm::vec4& color = glm::vec4(1.0f),
                       const glm::vec4& texCoords = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
                       float tilingFactor = 1.0f);

        /// Create default quad geometry
        static SpriteQuad CreateDefaultQuad();

        /// Get current renderer state
        RendererState GetState() const { return m_State; }
        
        /// Get renderer statistics
        const RendererStatistics& GetStatistics() const { return m_Statistics; }
        
        /// Reset statistics
        void ResetStatistics();
        
        /// Check if renderer is in error state
        bool IsInErrorState() const { return m_State == RendererState::Error; }
        
        /// Attempt to recover from error state
        bool TryRecover();
        
        /// Get last error message
        const std::string& GetLastError() const { return m_LastError; }

    private:
        /// Validate renderer state for operations
        bool ValidateState(RendererState requiredState, const std::string& operation);
        
        /// Set error state with message
        void SetError(const std::string& error);
        
        /// Update frame timing statistics
        void UpdateFrameStatistics();

        void* m_Device{ nullptr };
        void* m_PhysicalDevice{ nullptr };
        void* m_RenderPass{ nullptr };
        glm::mat4 m_ViewProjection{ 1.0f };
        
        RendererState m_State{ RendererState::Uninitialized };
        std::string m_LastError;
        RendererStatistics m_Statistics;
        
        std::chrono::high_resolution_clock::time_point m_FrameStartTime;
        bool m_FrameActive{ false };
    };

} // namespace vyra::renderer