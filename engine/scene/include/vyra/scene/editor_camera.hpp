#pragma once

#include "vyra/core/base.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace vyra::scene {

    // Stateless input snapshot passed per frame from the platform layer
    struct CameraInput {
        float MouseDeltaX{ 0.0f };
        float MouseDeltaY{ 0.0f };
        float ScrollDelta{ 0.0f };
        bool RightMouseDown{ false };
        bool MiddleMouseDown{ false };
        bool ShiftHeld{ false };
    };

    class VYRA_API EditorCamera {
    public:
        EditorCamera() = default;
        EditorCamera(float fovDegrees, float aspectRatio, float nearClip, float farClip);

        void OnUpdate(const CameraInput& input, float deltaTime);

        void SetViewportSize(float width, float height);

        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        glm::mat4 GetViewProjection() const { return m_ProjectionMatrix * m_ViewMatrix; }

        glm::vec3 GetForwardDirection() const;
        glm::vec3 GetRightDirection() const;
        glm::vec3 GetUpDirection() const;

        const glm::vec3& GetPosition() const { return m_Position; }
        const glm::quat& GetOrientation() const;

        float GetPitch() const { return m_Pitch; }
        float GetYaw() const { return m_Yaw; }

        float GetNearClip() const { return m_NearClip; }
        float GetFarClip() const { return m_FarClip; }
        float GetFOV() const { return m_FOV; }

    private:
        void RecalculateView();
        void RecalculateProjection();

        glm::vec3 CalculatePosition() const;
        glm::quat CalculateOrientation() const;

    private:
        float m_FOV{ glm::radians(45.0f) };
        float m_AspectRatio{ 16.0f / 9.0f };
        float m_NearClip{ 0.1f };
        float m_FarClip{ 1000.0f };

        glm::mat4 m_ViewMatrix{ 1.0f };
        glm::mat4 m_ProjectionMatrix{ 1.0f };

        glm::vec3 m_FocalPoint{ 0.0f, 0.0f, 0.0f };
        glm::vec3 m_Position{ 0.0f, 0.0f, 0.0f };

        float m_Distance{ 8.0f };
        float m_Pitch{ 0.3f };
        float m_Yaw{ -0.3f };

        float m_ViewportWidth{ 1280.0f };
        float m_ViewportHeight{ 720.0f };

        float m_PanSpeed{ 0.008f };
        float m_RotationSpeed{ 0.8f };
        float m_ZoomSpeed{ 1.5f };
    };

} // namespace vyra::scene
