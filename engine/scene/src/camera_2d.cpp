#include "vyra/scene/camera_2d.hpp"
#include "vyra/core/log.hpp"

namespace vyra::scene {

    Camera2D::Camera2D() {
        RecalculateProjectionMatrix();
        RecalculateViewMatrix();
    }

    void Camera2D::SetOrthographic(float size, float nearClip, float farClip) {
        m_ProjectionType = ProjectionType::Orthographic;
        m_OrthographicSize = size;
        m_OrthographicNear = nearClip;
        m_OrthographicFar = farClip;
        RecalculateProjectionMatrix();
    }

    void Camera2D::SetViewportSize(uint32_t width, uint32_t height) {
        m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
        RecalculateProjectionMatrix();
    }

    void Camera2D::RecalculateProjectionMatrix() {
        if (m_ProjectionType == ProjectionType::Orthographic) {
            float orthoLeft = -m_OrthographicSize * m_AspectRatio * m_Zoom;
            float orthoRight = m_OrthographicSize * m_AspectRatio * m_Zoom;
            float orthoBottom = -m_OrthographicSize * m_Zoom;
            float orthoTop = m_OrthographicSize * m_Zoom;

            m_ProjectionMatrix = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, 
                                          m_OrthographicNear, m_OrthographicFar);
        }

        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void Camera2D::RecalculateViewMatrix() {
        // Create view matrix from position and rotation
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(m_Position, 0.0f));
        transform = glm::rotate(transform, m_Rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        
        // View matrix is inverse of camera transform
        m_ViewMatrix = glm::inverse(transform);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

} // namespace vyra::scene