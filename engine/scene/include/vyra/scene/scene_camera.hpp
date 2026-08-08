#pragma once

#include "vyra/core/base.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vyra::scene {

    class VYRA_API SceneCamera {
    public:
        enum class ProjectionType { Perspective = 0, Orthographic = 1 };

    public:
        SceneCamera() { UpdateProjection(); }
        ~SceneCamera() = default;

        void SetPerspective(float verticalFov, float nearClip, float farClip) {
            m_ProjectionType = ProjectionType::Perspective;
            m_PerspectiveFOV = verticalFov;
            m_PerspectiveNear = nearClip;
            m_PerspectiveFar = farClip;
            UpdateProjection();
        }

        void SetOrthographic(float size, float nearClip, float farClip) {
            m_ProjectionType = ProjectionType::Orthographic;
            m_OrthographicSize = size;
            m_OrthographicNear = nearClip;
            m_OrthographicFar = farClip;
            UpdateProjection();
        }

        void SetViewportSize(uint32_t width, uint32_t height) {
            if (width == 0 || height == 0) return;
            m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
            UpdateProjection();
        }

        ProjectionType GetProjectionType() const { return m_ProjectionType; }
        void SetProjectionType(ProjectionType type) { m_ProjectionType = type; UpdateProjection(); }

        float GetPerspectiveVerticalFOV() const { return m_PerspectiveFOV; }
        void SetPerspectiveVerticalFOV(float verticalFov) { m_PerspectiveFOV = verticalFov; UpdateProjection(); }
        float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
        void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; UpdateProjection(); }
        float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
        void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; UpdateProjection(); }

        float GetOrthographicSize() const { return m_OrthographicSize; }
        void SetOrthographicSize(float size) { m_OrthographicSize = size; UpdateProjection(); }
        float GetOrthographicNearClip() const { return m_OrthographicNear; }
        void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; UpdateProjection(); }
        float GetOrthographicFarClip() const { return m_OrthographicFar; }
        void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; UpdateProjection(); }

        const glm::mat4& GetProjection() const { return m_Projection; }

    private:
        void UpdateProjection() {
            if (m_ProjectionType == ProjectionType::Perspective) {
                m_Projection = glm::perspective(m_PerspectiveFOV, m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
            } else {
                float orthoLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
                float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
                float orthoBottom = -m_OrthographicSize * 0.5f;
                float orthoTop = m_OrthographicSize * 0.5f;
                m_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
            }
        }

    private:
        ProjectionType m_ProjectionType{ ProjectionType::Perspective };

        float m_PerspectiveFOV{ glm::radians(45.0f) };
        float m_PerspectiveNear{ 0.1f };
        float m_PerspectiveFar{ 1000.0f };

        float m_OrthographicSize{ 10.0f };
        float m_OrthographicNear{ -1.0f };
        float m_OrthographicFar{ 1.0f };

        float m_AspectRatio{ 1.7777778f }; // 16:9
        glm::mat4 m_Projection{ 1.0f };
    };

} // namespace vyra::scene
