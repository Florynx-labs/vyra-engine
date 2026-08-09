#pragma once

#include "vyra/core/base.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vyra::scene {

    // 2D Camera component with orthographic projection
    class VYRA_API Camera2D {
    public:
        Camera2D();
        ~Camera2D() = default;

        // Projection settings
        void SetOrthographic(float size, float nearClip, float farClip);
        void SetViewportSize(uint32_t width, uint32_t height);

        // Camera properties
        const glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
        const glm::mat4& GetView() const { return m_ViewMatrix; }
        const glm::mat4& GetViewProjection() const { return m_ViewProjectionMatrix; }

        // Position
        const glm::vec2& GetPosition() const { return m_Position; }
        void SetPosition(const glm::vec2& position) { m_Position = position; RecalculateViewMatrix(); }

        // Rotation
        float GetRotation() const { return m_Rotation; }
        void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

        // Zoom
        float GetZoom() const { return m_Zoom; }
        void SetZoom(float zoom) { m_Zoom = zoom; RecalculateProjectionMatrix(); }

        // Projection type
        enum class ProjectionType { Orthographic = 0 };
        ProjectionType GetProjectionType() const { return m_ProjectionType; }

        // Orthographic properties
        float GetOrthographicSize() const { return m_OrthographicSize; }
        float GetOrthographicNearClip() const { return m_OrthographicNear; }
        float GetOrthographicFarClip() const { return m_OrthographicFar; }

    private:
        void RecalculateProjectionMatrix();
        void RecalculateViewMatrix();

    private:
        glm::mat4 m_ProjectionMatrix{ 1.0f };
        glm::mat4 m_ViewMatrix{ 1.0f };
        glm::mat4 m_ViewProjectionMatrix{ 1.0f };

        glm::vec2 m_Position{ 0.0f, 0.0f };
        float m_Rotation{ 0.0f }; // In radians
        float m_Zoom{ 1.0f };

        ProjectionType m_ProjectionType{ ProjectionType::Orthographic };

        // Orthographic settings
        float m_OrthographicSize{ 10.0f };
        float m_OrthographicNear{ -1.0f };
        float m_OrthographicFar{ 1.0f };

        float m_AspectRatio{ 1.0f };
    };

} // namespace vyra::scene