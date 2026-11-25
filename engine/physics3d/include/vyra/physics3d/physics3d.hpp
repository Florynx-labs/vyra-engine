#pragma once

#include "vyra/core/base.hpp"

#include <glm/vec3.hpp>
#include <optional>

namespace vyra::physics3d {

struct AABB {
    glm::vec3 Min{0.0f};
    glm::vec3 Max{0.0f};
    bool IsValid() const;
    glm::vec3 Center() const;
    glm::vec3 HalfExtents() const;
};

struct Sphere {
    glm::vec3 Center{0.0f};
    float Radius{0.5f};
    bool IsValid() const;
};

struct Contact {
    glm::vec3 Normal{0.0f};
    float Penetration{0.0f};
    glm::vec3 Point{0.0f};
};

struct Ray3D {
    glm::vec3 Origin{0.0f};
    glm::vec3 Direction{0.0f, 0.0f, -1.0f};
    float MaxDistance{1000.0f};
    bool IsValid() const;
};

struct RayHit {
    glm::vec3 Point{0.0f};
    glm::vec3 Normal{0.0f};
    float Distance{0.0f};
};

VYRA_API bool Overlap(const AABB& first, const AABB& second, Contact* contact = nullptr);
VYRA_API bool Overlap(const Sphere& first, const Sphere& second, Contact* contact = nullptr);
VYRA_API bool Overlap(const Sphere& sphere, const AABB& box, Contact* contact = nullptr);
VYRA_API bool Overlap(const AABB& box, const Sphere& sphere, Contact* contact = nullptr);
VYRA_API std::optional<RayHit> Raycast(const Ray3D& ray, const AABB& box);
VYRA_API std::optional<RayHit> Raycast(const Ray3D& ray, const Sphere& sphere);

} // namespace vyra::physics3d
