#pragma once

#include "vyra/core/base.hpp"

#include <glm/vec2.hpp>
#include <optional>

namespace vyra::physics2d {

/// Axis-aligned box in world space. Min must be <= Max on both axes.
struct AABB {
    glm::vec2 Min{0.0f};
    glm::vec2 Max{0.0f};

    bool IsValid() const;
    glm::vec2 Center() const;
    glm::vec2 HalfExtents() const;
};

/// Circle collider in world space.
struct Circle {
    glm::vec2 Center{0.0f};
    float Radius{0.5f};

    bool IsValid() const;
};

/// A contact normal points from the first shape toward the second shape.
struct Contact {
    glm::vec2 Normal{0.0f};
    float Penetration{0.0f};
    glm::vec2 Point{0.0f};
};

/// A ray uses a normalized direction and a non-negative maximum distance.
struct Ray2D {
    glm::vec2 Origin{0.0f};
    glm::vec2 Direction{1.0f, 0.0f};
    float MaxDistance{1000.0f};

    bool IsValid() const;
};

struct RayHit {
    glm::vec2 Point{0.0f};
    glm::vec2 Normal{0.0f};
    float Distance{0.0f};
};

VYRA_API bool Overlap(const AABB& first, const AABB& second, Contact* contact = nullptr);
VYRA_API bool Overlap(const Circle& first, const Circle& second, Contact* contact = nullptr);
VYRA_API bool Overlap(const Circle& circle, const AABB& box, Contact* contact = nullptr);
VYRA_API bool Overlap(const AABB& box, const Circle& circle, Contact* contact = nullptr);

VYRA_API std::optional<RayHit> Raycast(const Ray2D& ray, const AABB& box);
VYRA_API std::optional<RayHit> Raycast(const Ray2D& ray, const Circle& circle);

} // namespace vyra::physics2d
