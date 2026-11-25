#pragma once

#include "vyra/physics2d/physics2d.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>

namespace vyra::physics2d {

/// Deterministic uniform-grid broadphase. Returned pairs are unique and ordered.
class VYRA_API SpatialHash2D {
public:
    explicit SpatialHash2D(float cellSize = 4.0f, std::size_t maxCellsPerBody = 4096);

    bool SetCellSize(float cellSize);
    float GetCellSize() const { return m_CellSize; }

    /// Rebuilds the grid. Invalid bounds and bodies spanning too many cells are skipped.
    void Rebuild(const std::vector<AABB>& bounds);
    std::vector<std::pair<std::uint32_t, std::uint32_t>> ComputePairs() const;
    void Clear();

private:
    using Cell = std::pair<std::int32_t, std::int32_t>;
    float m_CellSize{4.0f};
    std::size_t m_MaxCellsPerBody{4096};
    std::map<Cell, std::vector<std::uint32_t>> m_Cells;
};

struct RigidBody2D {
    glm::vec2 Position{0.0f};
    glm::vec2 Velocity{0.0f};
    glm::vec2 Force{0.0f};
    float Rotation{0.0f};
    float AngularVelocity{0.0f};
    float Mass{1.0f}; // Zero means static.
    float LinearDamping{0.0f};
    float AngularDamping{0.0f};
    bool UseGravity{true};
    bool Enabled{true};
    glm::vec2 HalfExtents{0.5f};
    float Friction{0.5f};
    float Restitution{0.0f};
    bool IsTrigger{false};

    bool IsValid() const;
    float InverseMass() const;
    void ApplyForce(const glm::vec2& force);
    void ApplyImpulse(const glm::vec2& impulse);
};

struct TriggerEvent2D {
    std::uint32_t FirstBody{0};
    std::uint32_t SecondBody{0};
    bool Entered{true};
};

struct DistanceJoint2D {
    std::uint32_t FirstBody{0};
    std::uint32_t SecondBody{0};
    float RestLength{1.0f};
    float Stiffness{1.0f};
    bool Enabled{true};
};

struct PhysicsWorldSettings2D {
    glm::vec2 Gravity{0.0f, -9.81f};
    float FixedTimeStep{1.0f / 60.0f};
    std::uint32_t MaxSubSteps{8};
    std::uint32_t SolverIterations{8};
    float PositionCorrection{0.2f};
};

/// Fixed-step semi-implicit Euler simulation. Collision resolution is a later layer.
class VYRA_API PhysicsWorld2D {
public:
    explicit PhysicsWorld2D(PhysicsWorldSettings2D settings = {});

    bool SetSettings(const PhysicsWorldSettings2D& settings);
    const PhysicsWorldSettings2D& GetSettings() const { return m_Settings; }

    /// Adds elapsed frame time and executes bounded fixed simulation steps.
    std::uint32_t Advance(float frameDelta, std::vector<RigidBody2D>& bodies);
    void Step(float fixedDelta, std::vector<RigidBody2D>& bodies);
    void SetJoints(std::vector<DistanceJoint2D> joints) { m_Joints = std::move(joints); }
    const std::vector<DistanceJoint2D>& GetJoints() const { return m_Joints; }
    const std::vector<TriggerEvent2D>& GetTriggerEvents() const { return m_TriggerEvents; }
    void ResetAccumulator() { m_Accumulator = 0.0f; }
    float GetInterpolationAlpha() const;

private:
    PhysicsWorldSettings2D m_Settings;
    float m_Accumulator{0.0f};
    std::vector<DistanceJoint2D> m_Joints;
    std::vector<TriggerEvent2D> m_TriggerEvents;
};

} // namespace vyra::physics2d
