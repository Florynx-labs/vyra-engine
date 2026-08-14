#include "vyra/renderer/sprite_renderer.hpp"
#include "vyra/core/log.hpp"

#define VK_NO_PROTOTYPES
#include <volk.h>

namespace vyra::renderer {

    bool SpriteRenderer::Init(void* device, void* physicalDevice, void* renderPass) {
        if (m_State != RendererState::Uninitialized) {
            SetError("SpriteRenderer already initialized");
            return false;
        }
        
        if (!device || !physicalDevice || !renderPass) {
            SetError("Invalid parameters for Init");
            return false;
        }
        
        m_Device = device;
        m_PhysicalDevice = physicalDevice;
        m_RenderPass = renderPass;
        
        m_State = RendererState::Ready;
        ResetStatistics();
        
        VYRA_LOG_CHANNEL(LogChannel::Renderer, info, "SpriteRenderer initialized successfully");
        return true;
    }

    void SpriteRenderer::Shutdown() {
        if (m_State == RendererState::Uninitialized) {
            return;
        }
        
        // Clean up resources
        m_Device = nullptr;
        m_PhysicalDevice = nullptr;
        m_RenderPass = nullptr;
        
        m_State = RendererState::Uninitialized;
        VYRA_LOG_CHANNEL(LogChannel::Renderer, info, "SpriteRenderer shut down");
    }

    bool SpriteRenderer::BeginScene(const glm::mat4& viewProjection) {
        if (!ValidateState(RendererState::Ready, "BeginScene")) {
            return false;
        }
        
        m_ViewProjection = viewProjection;
        m_State = RendererState::SceneActive;
        m_FrameActive = true;
        m_FrameStartTime = std::chrono::high_resolution_clock::now();
        
        VYRA_LOG_CHANNEL(LogChannel::Renderer, debug, "SpriteRenderer scene begun");
        return true;
    }

    bool SpriteRenderer::EndScene() {
        if (!ValidateState(RendererState::SceneActive, "EndScene")) {
            return false;
        }
        
        // Update frame statistics
        UpdateFrameStatistics();
        
        m_State = RendererState::Ready;
        m_FrameActive = false;
        
        VYRA_LOG_CHANNEL(LogChannel::Renderer, debug, "SpriteRenderer scene ended");
        return true;
    }

    bool SpriteRenderer::DrawSprite(void* commandBuffer, const glm::mat4& transform,
                                    const glm::vec4& color, const glm::vec4& texCoords,
                                    float tilingFactor) {
        if (!ValidateState(RendererState::SceneActive, "DrawSprite")) {
            return false;
        }
        
        if (!commandBuffer) {
            SetError("Invalid command buffer in DrawSprite");
            return false;
        }
        
        // TODO: Implement actual sprite rendering
        // This would:
        // 1. Set up push constants
        // 2. Bind sprite pipeline
        // 3. Bind vertex/index buffers
        // 4. Draw indexed
        
        // Update statistics
        m_Statistics.DrawCalls++;
        m_Statistics.SpritesDrawn++;
        m_Statistics.VerticesProcessed += 4; // 4 vertices per sprite
        
        (void)transform;
        (void)color;
        (void)texCoords;
        (void)tilingFactor;
        
        return true;
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
    
    void SpriteRenderer::ResetStatistics() {
        m_Statistics = RendererStatistics{};
    }
    
    bool SpriteRenderer::TryRecover() {
        if (m_State != RendererState::Error) {
            return true; // Not in error state
        }
        
        VYRA_LOG_CHANNEL(LogChannel::Renderer, info, "Attempting to recover SpriteRenderer from error state");
        
        // Reset state to ready
        m_State = RendererState::Ready;
        m_LastError.clear();
        
        // Reset frame state
        m_FrameActive = false;
        
        VYRA_LOG_CHANNEL(LogChannel::Renderer, info, "SpriteRenderer recovered successfully");
        return true;
    }
    
    bool SpriteRenderer::ValidateState(RendererState requiredState, const std::string& operation) {
        if (m_State == RendererState::Error) {
            VYRA_LOG_CHANNEL(LogChannel::Renderer, error, "Cannot perform '{0}' - renderer in error state: {1}", 
                             operation, m_LastError);
            return false;
        }
        
        if (m_State != requiredState) {
            std::string error = "Invalid state for operation '" + operation + 
                             "'. Required: " + std::to_string(static_cast<int>(requiredState)) + 
                             ", Current: " + std::to_string(static_cast<int>(m_State));
            SetError(error);
            return false;
        }
        
        return true;
    }
    
    void SpriteRenderer::SetError(const std::string& error) {
        m_LastError = error;
        m_State = RendererState::Error;
        VYRA_LOG_CHANNEL(LogChannel::Renderer, error, "SpriteRenderer error: {0}", error);
    }
    
    void SpriteRenderer::UpdateFrameStatistics() {
        if (!m_FrameActive) {
            return;
        }
        
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_FrameStartTime);
        double frameTimeMs = duration.count() / 1000.0;
        
        m_Statistics.FrameTimeMs = frameTimeMs;
        m_Statistics.FrameCount++;
        
        // Update average frame time
        if (m_Statistics.FrameCount == 1) {
            m_Statistics.AverageFrameTimeMs = frameTimeMs;
        } else {
            // Running average
            m_Statistics.AverageFrameTimeMs = 
                (m_Statistics.AverageFrameTimeMs * (m_Statistics.FrameCount - 1) + frameTimeMs) / 
                m_Statistics.FrameCount;
        }
    }

} // namespace vyra::renderer