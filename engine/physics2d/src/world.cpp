#include "vyra/physics2d/world.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

namespace vyra::physics2d {
namespace {
bool Finite(float value) { return std::isfinite(value); }
bool Finite(const glm::vec2& value) { return Finite(value.x) && Finite(value.y); }
}

SpatialHash2D::SpatialHash2D(float cellSize, std::size_t maxCellsPerBody)
    : m_MaxCellsPerBody(std::max<std::size_t>(1, maxCellsPerBody)) {
    SetCellSize(cellSize);
}

bool SpatialHash2D::SetCellSize(float cellSize) {
    if (!Finite(cellSize) || cellSize <= 0.0f) return false;
    m_CellSize = cellSize;
    Clear();
    return true;
}

void SpatialHash2D::Clear() {
    m_Cells.clear();
}

void SpatialHash2D::Rebuild(const std::vector<AABB>& bounds) {
    Clear();
    for (std::size_t index = 0; index < bounds.size(); ++index) {
        if (index > std::numeric_limits<std::uint32_t>::max()) break;
        const AABB& box = bounds[index];
        if (!box.IsValid()) continue;

        const double minX = std::floor(static_cast<double>(box.Min.x) / m_CellSize);
        const double minY = std::floor(static_cast<double>(box.Min.y) / m_CellSize);
        const double maxX = std::floor(static_cast<double>(box.Max.x) / m_CellSize);
        const double maxY = std::floor(static_cast<double>(box.Max.y) / m_CellSize);
        constexpr double low = static_cast<double>(std::numeric_limits<std::int32_t>::min());
        constexpr double high = static_cast<double>(std::numeric_limits<std::int32_t>::max());
        if (minX < low || minY < low || maxX > high || maxY > high) continue;

        const auto x0 = static_cast<std::int32_t>(minX);
        const auto y0 = static_cast<std::int32_t>(minY);
        const auto x1 = static_cast<std::int32_t>(maxX);
        const auto y1 = static_cast<std::int32_t>(maxY);
        const std::uint64_t width = static_cast<std::uint64_t>(
            static_cast<std::int64_t>(x1) - x0 + 1);
        const std::uint64_t height = static_cast<std::uint64_t>(
            static_cast<std::int64_t>(y1) - y0 + 1);
        if (width == 0 || height == 0 ||
            width > m_MaxCellsPerBody || height > m_MaxCellsPerBody ||
            width * height > m_MaxCellsPerBody) continue;

        for (std::int64_t x = x0; x <= x1; ++x) {
            for (std::int64_t y = y0; y <= y1; ++y) {
                m_Cells[{static_cast<std::int32_t>(x), static_cast<std::int32_t>(y)}]
                    .push_back(static_cast<std::uint32_t>(index));
            }
        }
    }
}

std::vector<std::pair<std::uint32_t, std::uint32_t>> SpatialHash2D::ComputePairs() const {
    std::set<std::pair<std::uint32_t, std::uint32_t>> unique;
    for (const auto& [cell, indices] : m_Cells) {
        (void)cell;
        for (std::size_t i = 0; i < indices.size(); ++i) {
            for (std::size_t j = i + 1; j < indices.size(); ++j) {
                const auto a = std::min(indices[i], indices[j]);
                const auto b = std::max(indices[i], indices[j]);
                if (a != b) unique.emplace(a, b);
            }
        }
    }
    return {unique.begin(), unique.end()};
}

bool RigidBody2D::IsValid() const {
    return Finite(Position) && Finite(Velocity) && Finite(Force) &&
        Finite(Rotation) && Finite(AngularVelocity) && Finite(Mass) &&
        Finite(LinearDamping) && Finite(AngularDamping) &&
        Mass >= 0.0f && LinearDamping >= 0.0f && AngularDamping >= 0.0f;
}

float RigidBody2D::InverseMass() const {
    return Mass > std::numeric_limits<float>::epsilon() ? 1.0f / Mass : 0.0f;
}

void RigidBody2D::ApplyForce(const glm::vec2& force) {
    if (Enabled && InverseMass() > 0.0f && Finite(force)) Force += force;
}

void RigidBody2D::ApplyImpulse(const glm::vec2& impulse) {
    if (Enabled && InverseMass() > 0.0f && Finite(impulse)) {
        Velocity += impulse * InverseMass();
    }
}

PhysicsWorld2D::PhysicsWorld2D(PhysicsWorldSettings2D settings) {
    SetSettings(settings);
}

bool PhysicsWorld2D::SetSettings(const PhysicsWorldSettings2D& settings) {
    if (!Finite(settings.Gravity) || !Finite(settings.FixedTimeStep) ||
        settings.FixedTimeStep <= 0.0f || settings.MaxSubSteps == 0) return false;
    m_Settings = settings;
    m_Accumulator = std::min(m_Accumulator, m_Settings.FixedTimeStep);
    return true;
}

std::uint32_t PhysicsWorld2D::Advance(float frameDelta, std::vector<RigidBody2D>& bodies) {
    if (!Finite(frameDelta) || frameDelta <= 0.0f) return 0;
    m_Accumulator += std::min(frameDelta,
        m_Settings.FixedTimeStep * static_cast<float>(m_Settings.MaxSubSteps));
    std::uint32_t steps = 0;
    while (m_Accumulator >= m_Settings.FixedTimeStep &&
           steps < m_Settings.MaxSubSteps) {
        Step(m_Settings.FixedTimeStep, bodies);
        m_Accumulator -= m_Settings.FixedTimeStep;
        ++steps;
    }
    return steps;
}

void PhysicsWorld2D::Step(float fixedDelta, std::vector<RigidBody2D>& bodies) {
    if (!Finite(fixedDelta) || fixedDelta <= 0.0f) return;
    for (RigidBody2D& body : bodies) {
        if (!body.Enabled || !body.IsValid() || body.InverseMass() == 0.0f) {
            body.Force = glm::vec2(0.0f);
            continue;
        }
        const glm::vec2 acceleration = body.Force * body.InverseMass() +
            (body.UseGravity ? m_Settings.Gravity : glm::vec2(0.0f));
        body.Velocity += acceleration * fixedDelta;
        body.Position += body.Velocity * fixedDelta;
        body.AngularVelocity *= 1.0f /
            (1.0f + body.AngularDamping * fixedDelta);
        body.Rotation += body.AngularVelocity * fixedDelta;
        body.Velocity *= 1.0f /
            (1.0f + body.LinearDamping * fixedDelta);
        body.Force = glm::vec2(0.0f);
    }
}

float PhysicsWorld2D::GetInterpolationAlpha() const {
    return m_Settings.FixedTimeStep > 0.0f
        ? std::clamp(m_Accumulator / m_Settings.FixedTimeStep, 0.0f, 1.0f)
        : 0.0f;
}

} // namespace vyra::physics2d
