#include "vyra/scene/editor_camera.hpp"
#include <algorithm>
#include <cmath>

namespace vyra::scene {

    EditorCamera::EditorCamera(float fovDegrees, float aspectRatio, float nearClip, float farClip)
        : m_FOV(glm::radians(fovDegrees))
        , m_AspectRatio(aspectRatio)
        , m_NearClip(nearClip)
        , m_FarClip(farClip)
    {
        RecalculateProjection();
        RecalculateView();
    }

    void EditorCamera::SetViewportSize(float width, float height) {
        if (m_ViewportWidth == width && m_ViewportHeight == height) return;
        m_ViewportWidth = width;
        m_ViewportHeight = height;
        m_AspectRatio = width / height;
        RecalculateProjection();
    }

    void EditorCamera::OnUpdate(const CameraInput& input, float deltaTime) {
        const float dt = deltaTime;

        // Orbit: Right Mouse Button drag
        if (input.RightMouseDown) {
            m_Yaw   += input.MouseDeltaX * m_RotationSpeed * dt;
            m_Pitch -= input.MouseDeltaY * m_RotationSpeed * dt;
            // Clamp pitch to avoid gimbal lock
            m_Pitch = std::clamp(m_Pitch, -glm::half_pi<float>() + 0.05f, glm::half_pi<float>() - 0.05f);
        }

        // Pan: Middle Mouse Button drag
        if (input.MiddleMouseDown) {
            float speedScale = std::max(m_Distance * 0.5f, 0.1f);
            m_FocalPoint -= GetRightDirection() * (input.MouseDeltaX * m_PanSpeed * speedScale);
            m_FocalPoint += GetUpDirection()    * (input.MouseDeltaY * m_PanSpeed * speedScale);
        }

        // Zoom: Mouse Scroll
        if (std::abs(input.ScrollDelta) > 0.0001f) {
            float speed = std::max(m_Distance * 0.2f, 0.1f);
            m_Distance -= input.ScrollDelta * speed * m_ZoomSpeed;
            m_Distance = std::max(m_Distance, 0.5f);
        }

        RecalculateView();
    }

    glm::quat EditorCamera::CalculateOrientation() const {
        return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
    }

    const glm::quat& EditorCamera::GetOrientation() const {
        static glm::quat orientation;
        orientation = CalculateOrientation();
        return orientation;
    }

    glm::vec3 EditorCamera::GetForwardDirection() const {
        return glm::rotate(CalculateOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::vec3 EditorCamera::GetRightDirection() const {
        return glm::rotate(CalculateOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 EditorCamera::GetUpDirection() const {
        return glm::rotate(CalculateOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 EditorCamera::CalculatePosition() const {
        return m_FocalPoint - GetForwardDirection() * m_Distance;
    }

    void EditorCamera::RecalculateView() {
        m_Position = CalculatePosition();
        glm::quat orientation = CalculateOrientation();
        m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
        m_ViewMatrix = glm::inverse(m_ViewMatrix);
    }

    void EditorCamera::RecalculateProjection() {
        m_ProjectionMatrix = glm::perspective(m_FOV, m_AspectRatio, m_NearClip, m_FarClip);
        // Vulkan clip-space fix: flip Y
        m_ProjectionMatrix[1][1] *= -1.0f;
    }

} // namespace vyra::scene
