#pragma once

#include "vyra/core/base.hpp"
#include "vyra/ecs/ecs.hpp"
#include "vyra/scene/scene_camera.hpp"
#include <glm/glm.hpp>
#include <string>

namespace vyra::scene {

    // Re-export core ECS components into scene namespace
    using TagComponent = vyra::ecs::TagComponent;
    using TransformComponent = vyra::ecs::TransformComponent;

    struct VYRA_API CameraComponent {
        SceneCamera Camera;
        bool Primary = true;
        bool FixedAspectRatio = false;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
    };

    struct VYRA_API MeshComponent {
        std::string AssetPath;
        glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        uint32_t MaterialID{ 0 };

        MeshComponent() = default;
        MeshComponent(const std::string& path) : AssetPath(path) {}
        MeshComponent(const std::string& path, const glm::vec4& color)
            : AssetPath(path), Color(color) {}
    };

    struct VYRA_API SpriteRendererComponent {
        glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        std::string TexturePath;
        float TilingFactor{ 1.0f };

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const glm::vec4& color) : Color(color) {}
    };

    struct VYRA_API DirectionalLightComponent {
        glm::vec3 Color{ 1.0f, 1.0f, 1.0f };
        float Intensity{ 1.0f };

        DirectionalLightComponent() = default;
    };

    struct VYRA_API PointLightComponent {
        glm::vec3 Color{ 1.0f, 1.0f, 1.0f };
        float Intensity{ 1.0f };
        float Radius{ 10.0f };

        PointLightComponent() = default;
    };

} // namespace vyra::scene
