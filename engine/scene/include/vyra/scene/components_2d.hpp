#pragma once

#include "vyra/core/base.hpp"
#include "vyra/ecs/ecs.hpp"
#include <glm/glm.hpp>
#include <string>

namespace vyra::scene {

    // 2D Transform component for 2D entities
    struct VYRA_API Transform2DComponent {
        glm::vec2 Position{ 0.0f, 0.0f };
        float Rotation{ 0.0f }; // Rotation in radians
        glm::vec2 Scale{ 1.0f, 1.0f };
        float ZOrder{ 0.0f }; // For 2D depth sorting

        Transform2DComponent() = default;
        Transform2DComponent(const glm::vec2& position) : Position(position) {}
        Transform2DComponent(const glm::vec2& position, float rotation, const glm::vec2& scale)
            : Position(position), Rotation(rotation), Scale(scale) {}

        // Get 2D transformation matrix
        glm::mat3 GetTransform() const {
            glm::mat3 translation = glm::mat3(1.0f);
            translation[2][0] = Position.x;
            translation[2][1] = Position.y;

            glm::mat3 rotation = glm::mat3(1.0f);
            float c = glm::cos(Rotation);
            float s = glm::sin(Rotation);
            rotation[0][0] = c;
            rotation[0][1] = -s;
            rotation[1][0] = s;
            rotation[1][1] = c;

            glm::mat3 scale = glm::mat3(1.0f);
            scale[0][0] = Scale.x;
            scale[1][1] = Scale.y;

            return translation * rotation * scale;
        }
    };

    // Sprite component for 2D rendering
    struct VYRA_API SpriteComponent {
        glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        std::string TexturePath;
        float TilingFactor{ 1.0f };
        
        // Sprite atlas support (future)
        glm::vec4 TexCoords{ 0.0f, 0.0f, 1.0f, 1.0f }; // x, y, width, height in UV space
        
        SpriteComponent() = default;
        SpriteComponent(const glm::vec4& color) : Color(color) {}
        SpriteComponent(const std::string& texturePath) : TexturePath(texturePath) {}
        SpriteComponent(const std::string& texturePath, const glm::vec4& color)
            : TexturePath(texturePath), Color(color) {}
    };

} // namespace vyra::scene