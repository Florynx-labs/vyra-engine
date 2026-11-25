#pragma once

#include "vyra/core/base.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cstdint>
#include <utility>
#include <vector>
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

struct Capsule {
    glm::vec3 A{0.0f, -0.5f, 0.0f};
    glm::vec3 B{0.0f, 0.5f, 0.0f};
    float Radius{0.5f};
    bool IsValid() const;
};

struct OBB {
    glm::vec3 Center{0.0f};
    glm::vec3 HalfExtents{0.5f};
    glm::quat Orientation{1.0f, 0.0f, 0.0f, 0.0f};
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

VYRA_API bool Overlap(const Capsule& first, const Capsule& second, Contact* contact = nullptr);
VYRA_API bool Overlap(const OBB& first, const OBB& second, Contact* contact = nullptr);
VYRA_API bool Overlap(const AABB& first, const AABB& second, Contact* contact = nullptr);
VYRA_API bool Overlap(const Sphere& first, const Sphere& second, Contact* contact = nullptr);
VYRA_API bool Overlap(const Sphere& sphere, const AABB& box, Contact* contact = nullptr);
VYRA_API bool Overlap(const AABB& box, const Sphere& sphere, Contact* contact = nullptr);
VYRA_API std::optional<RayHit> Raycast(const Ray3D& ray, const AABB& box);
VYRA_API std::optional<RayHit> Raycast(const Ray3D& ray, const Sphere& sphere);

class VYRA_API Broadphase3D {
public:
    // Sweep-and-prune on the X axis; candidate pairs are sorted and unique.
    std::vector<std::pair<std::uint32_t, std::uint32_t>>
    ComputePairs(const std::vector<AABB>& bounds) const;
};

struct RigidBody3D {
    glm::vec3 Position{0.0f};
    glm::vec3 Velocity{0.0f};
    glm::vec3 Force{0.0f};
    float Mass{1.0f};
    float Restitution{0.0f};
    bool UseGravity{true};
    bool Enabled{true};
    float InverseMass() const;
    bool IsValid() const;
    void ApplyForce(const glm::vec3& force);
    void ApplyImpulse(const glm::vec3& impulse);
};

struct PhysicsWorldSettings3D {
    glm::vec3 Gravity{0.0f, -9.81f, 0.0f};
    float FixedTimeStep{1.0f / 60.0f};
    std::uint32_t MaxSubSteps{8};
};

class VYRA_API PhysicsWorld3D {
public:
    explicit PhysicsWorld3D(PhysicsWorldSettings3D settings = {});
    bool SetSettings(const PhysicsWorldSettings3D& settings);
    std::uint32_t Advance(float frameDelta, std::vector<RigidBody3D>& bodies);
    void Step(float fixedDelta, std::vector<RigidBody3D>& bodies);
    float GetInterpolationAlpha() const;
private:
    PhysicsWorldSettings3D m_Settings;
    float m_Accumulator{0.0f};
};

} // namespace vyra::physics3d
