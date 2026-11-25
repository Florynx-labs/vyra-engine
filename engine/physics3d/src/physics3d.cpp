#include "vyra/physics3d/physics3d.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace vyra::physics3d {
namespace {
bool Finite(const glm::vec3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
glm::vec3 SafeNormal(const glm::vec3& v, const glm::vec3& fallback) {
    const float d = glm::dot(v, v);
    return d <= std::numeric_limits<float>::epsilon() ? fallback : v / std::sqrt(d);
}
}

bool AABB::IsValid() const {
    return Finite(Min) && Finite(Max) && Min.x <= Max.x &&
        Min.y <= Max.y && Min.z <= Max.z;
}
glm::vec3 AABB::Center() const { return (Min + Max) * 0.5f; }
glm::vec3 AABB::HalfExtents() const { return (Max - Min) * 0.5f; }
bool Sphere::IsValid() const {
    return Finite(Center) && std::isfinite(Radius) && Radius >= 0.0f;
}
bool Ray3D::IsValid() const {
    const float d = glm::dot(Direction, Direction);
    return Finite(Origin) && Finite(Direction) && std::isfinite(MaxDistance) &&
        MaxDistance >= 0.0f && std::abs(d - 1.0f) <= 1.0e-3f;
}

bool Overlap(const AABB& a, const AABB& b, Contact* contact) {
    if (!a.IsValid() || !b.IsValid()) return false;
    const glm::vec3 delta = b.Center() - a.Center();
    const glm::vec3 depth = a.HalfExtents() + b.HalfExtents() - glm::abs(delta);
    if (depth.x < 0.0f || depth.y < 0.0f || depth.z < 0.0f) return false;
    if (contact) {
        int axis = depth.x < depth.y ? 0 : 1;
        if (depth.z < depth[axis]) axis = 2;
        contact->Normal = glm::vec3(0.0f);
        contact->Normal[axis] = delta[axis] < 0.0f ? -1.0f : 1.0f;
        contact->Penetration = depth[axis];
        contact->Point = (glm::max(a.Min, b.Min) + glm::min(a.Max, b.Max)) * 0.5f;
    }
    return true;
}

bool Overlap(const Sphere& a, const Sphere& b, Contact* contact) {
    if (!a.IsValid() || !b.IsValid()) return false;
    const glm::vec3 delta = b.Center - a.Center;
    const float d2 = glm::dot(delta, delta);
    const float sum = a.Radius + b.Radius;
    if (d2 > sum * sum) return false;
    if (contact) {
        const float distance = std::sqrt(std::max(d2, 0.0f));
        contact->Normal = SafeNormal(delta, glm::vec3(1.0f, 0.0f, 0.0f));
        contact->Penetration = sum - distance;
        contact->Point = a.Center + contact->Normal *
            (a.Radius - contact->Penetration * 0.5f);
    }
    return true;
}

bool Overlap(const Sphere& sphere, const AABB& box, Contact* contact) {
    if (!sphere.IsValid() || !box.IsValid()) return false;
    const glm::vec3 closest = glm::clamp(sphere.Center, box.Min, box.Max);
    const glm::vec3 delta = closest - sphere.Center;
    const float d2 = glm::dot(delta, delta);
    if (d2 > sphere.Radius * sphere.Radius) return false;
    if (contact) {
        const float distance = std::sqrt(std::max(d2, 0.0f));
        if (distance > std::numeric_limits<float>::epsilon()) {
            contact->Normal = delta / distance;
            contact->Penetration = sphere.Radius - distance;
            contact->Point = closest;
        } else {
            const glm::vec3 toMin = sphere.Center - box.Min;
            const glm::vec3 toMax = box.Max - sphere.Center;
            const float distances[6] = {toMin.x, toMax.x, toMin.y, toMax.y, toMin.z, toMax.z};
            int face = 0;
            for (int i = 1; i < 6; ++i) if (distances[i] < distances[face]) face = i;
            contact->Normal = glm::vec3(0.0f);
            contact->Normal[face / 2] = (face % 2 == 0) ? -1.0f : 1.0f;
            contact->Penetration = sphere.Radius + distances[face];
            contact->Point = sphere.Center + contact->Normal * distances[face];
        }
    }
    return true;
}
bool Overlap(const AABB& box, const Sphere& sphere, Contact* contact) {
    const bool hit = Overlap(sphere, box, contact);
    if (hit && contact) contact->Normal = -contact->Normal;
    return hit;
}

std::optional<RayHit> Raycast(const Ray3D& ray, const AABB& box) {
    if (!ray.IsValid() || !box.IsValid()) return std::nullopt;
    float nearT = 0.0f, farT = ray.MaxDistance;
    glm::vec3 normal{0.0f};
    for (int axis = 0; axis < 3; ++axis) {
        const float o = ray.Origin[axis], d = ray.Direction[axis];
        if (std::abs(d) <= std::numeric_limits<float>::epsilon()) {
            if (o < box.Min[axis] || o > box.Max[axis]) return std::nullopt;
            continue;
        }
        float t0 = (box.Min[axis] - o) / d;
        float t1 = (box.Max[axis] - o) / d;
        float sign = -1.0f;
        if (t0 > t1) { std::swap(t0, t1); sign = 1.0f; }
        if (t0 > nearT) {
            nearT = t0;
            normal = glm::vec3(0.0f);
            normal[axis] = sign;
        }
        farT = std::min(farT, t1);
        if (nearT > farT) return std::nullopt;
    }
    if (farT < 0.0f || nearT > ray.MaxDistance) return std::nullopt;
    const float distance = std::max(nearT, 0.0f);
    return RayHit{ray.Origin + ray.Direction * distance, normal, distance};
}

std::optional<RayHit> Raycast(const Ray3D& ray, const Sphere& sphere) {
    if (!ray.IsValid() || !sphere.IsValid()) return std::nullopt;
    const glm::vec3 offset = ray.Origin - sphere.Center;
    const float b = glm::dot(offset, ray.Direction);
    const float c = glm::dot(offset, offset) - sphere.Radius * sphere.Radius;
    const float discriminant = b * b - c;
    if (discriminant < 0.0f) return std::nullopt;
    const float root = std::sqrt(discriminant);
    float t = -b - root;
    if (t < 0.0f) t = -b + root;
    if (t < 0.0f || t > ray.MaxDistance) return std::nullopt;
    const glm::vec3 point = ray.Origin + ray.Direction * t;
    return RayHit{point, SafeNormal(point - sphere.Center, -ray.Direction), t};
}

bool Capsule::IsValid() const {
    return Finite(A) && Finite(B) && std::isfinite(Radius) && Radius >= 0.0f;
}
bool OBB::IsValid() const {
    const float q2 = glm::dot(Orientation, Orientation);
    return Finite(Center) && Finite(HalfExtents) &&
        HalfExtents.x >= 0.0f && HalfExtents.y >= 0.0f && HalfExtents.z >= 0.0f &&
        std::isfinite(q2) && q2 > 1.0e-8f;
}

bool Overlap(const OBB& a, const OBB& b, Contact* contact) {
    if (!a.IsValid() || !b.IsValid()) return false;
    const glm::mat3 ra = glm::mat3_cast(glm::normalize(a.Orientation));
    const glm::mat3 rb = glm::mat3_cast(glm::normalize(b.Orientation));
    const glm::vec3 axesA[3] = {ra[0], ra[1], ra[2]};
    const glm::vec3 axesB[3] = {rb[0], rb[1], rb[2]};
    const glm::vec3 delta = b.Center - a.Center;
    float minOverlap = std::numeric_limits<float>::max();
    glm::vec3 bestAxis{1.0f, 0.0f, 0.0f};
    auto testAxis = [&](glm::vec3 axis) {
        const float length2 = glm::dot(axis, axis);
        if (length2 < 1.0e-8f) return true;
        axis /= std::sqrt(length2);
        float raProjection = 0.0f, rbProjection = 0.0f;
        for (int i = 0; i < 3; ++i) {
            raProjection += a.HalfExtents[i] * std::abs(glm::dot(axis, axesA[i]));
            rbProjection += b.HalfExtents[i] * std::abs(glm::dot(axis, axesB[i]));
        }
        const float overlap = raProjection + rbProjection - std::abs(glm::dot(delta, axis));
        if (overlap < 0.0f) return false;
        if (overlap < minOverlap) {
            minOverlap = overlap;
            bestAxis = glm::dot(delta, axis) < 0.0f ? -axis : axis;
        }
        return true;
    };
    for (const auto& axis : axesA) if (!testAxis(axis)) return false;
    for (const auto& axis : axesB) if (!testAxis(axis)) return false;
    for (const auto& axisA : axesA)
        for (const auto& axisB : axesB)
            if (!testAxis(glm::cross(axisA, axisB))) return false;
    if (contact) {
        contact->Normal = bestAxis;
        contact->Penetration = minOverlap;
        contact->Point = (a.Center + b.Center) * 0.5f;
    }
    return true;
}

bool Overlap(const Capsule& a, const Capsule& b, Contact* contact) {
    if (!a.IsValid() || !b.IsValid()) return false;
    const glm::vec3 d1 = a.B - a.A, d2 = b.B - b.A, r = a.A - b.A;
    const float aa = glm::dot(d1, d1), ee = glm::dot(d2, d2), ff = glm::dot(d2, r);
    float u = 0.0f, v = 0.0f;
    if (aa <= 1.0e-8f && ee <= 1.0e-8f) {
        u = v = 0.0f;
    } else if (aa <= 1.0e-8f) {
        v = std::clamp(ff / ee, 0.0f, 1.0f);
    } else {
        const float cc = glm::dot(d1, r);
        if (ee <= 1.0e-8f) u = std::clamp(-cc / aa, 0.0f, 1.0f);
        else {
            const float bb = glm::dot(d1, d2);
            const float denom = aa * ee - bb * bb;
            if (denom > 1.0e-8f) u = std::clamp((bb * ff - cc * ee) / denom, 0.0f, 1.0f);
            v = (bb * u + ff) / ee;
            if (v < 0.0f) { v = 0.0f; u = std::clamp(-cc / aa, 0.0f, 1.0f); }
            else if (v > 1.0f) { v = 1.0f; u = std::clamp((bb - cc) / aa, 0.0f, 1.0f); }
        }
    }
    const glm::vec3 p = a.A + d1 * u, q = b.A + d2 * v;
    const glm::vec3 delta = q - p;
    const float distance2 = glm::dot(delta, delta), radius = a.Radius + b.Radius;
    if (distance2 > radius * radius) return false;
    if (contact) {
        const float distance = std::sqrt(std::max(distance2, 0.0f));
        contact->Normal = SafeNormal(delta, glm::vec3(1.0f, 0.0f, 0.0f));
        contact->Penetration = radius - distance;
        contact->Point = (p + q) * 0.5f;
    }
    return true;
}

std::vector<std::pair<std::uint32_t, std::uint32_t>>
Broadphase3D::ComputePairs(const std::vector<AABB>& bounds) const {
    std::vector<std::uint32_t> order;
    order.reserve(bounds.size());
    for (std::size_t i = 0; i < bounds.size() && i <= std::numeric_limits<std::uint32_t>::max(); ++i)
        if (bounds[i].IsValid()) order.push_back(static_cast<std::uint32_t>(i));
    std::sort(order.begin(), order.end(), [&](auto lhs, auto rhs) {
        return bounds[lhs].Min.x == bounds[rhs].Min.x ? lhs < rhs :
            bounds[lhs].Min.x < bounds[rhs].Min.x;
    });
    std::vector<std::pair<std::uint32_t, std::uint32_t>> pairs;
    for (std::size_t i = 0; i < order.size(); ++i) {
        const auto a = order[i];
        for (std::size_t j = i + 1; j < order.size(); ++j) {
            const auto b = order[j];
            if (bounds[b].Min.x > bounds[a].Max.x) break;
            if (bounds[a].Min.y <= bounds[b].Max.y && bounds[a].Max.y >= bounds[b].Min.y &&
                bounds[a].Min.z <= bounds[b].Max.z && bounds[a].Max.z >= bounds[b].Min.z)
                pairs.emplace_back(std::min(a, b), std::max(a, b));
        }
    }
    std::sort(pairs.begin(), pairs.end());
    return pairs;
}

float RigidBody3D::InverseMass() const {
    return Mass > std::numeric_limits<float>::epsilon() ? 1.0f / Mass : 0.0f;
}
bool RigidBody3D::IsValid() const {
    return Finite(Position) && Finite(Velocity) && Finite(Force) && Finite(HalfExtents) &&
        HalfExtents.x >= 0.0f && HalfExtents.y >= 0.0f && HalfExtents.z >= 0.0f &&
        std::isfinite(Mass) && Mass >= 0.0f &&
        std::isfinite(Restitution) && Restitution >= 0.0f && Restitution <= 1.0f &&
        std::isfinite(Friction) && Friction >= 0.0f && Friction <= 1.0f;
}
void RigidBody3D::ApplyForce(const glm::vec3& force) {
    if (Enabled && InverseMass() > 0.0f && Finite(force)) Force += force;
}
void RigidBody3D::ApplyImpulse(const glm::vec3& impulse) {
    if (Enabled && InverseMass() > 0.0f && Finite(impulse)) Velocity += impulse * InverseMass();
}

PhysicsWorld3D::PhysicsWorld3D(PhysicsWorldSettings3D settings) { SetSettings(settings); }
bool PhysicsWorld3D::SetSettings(const PhysicsWorldSettings3D& settings) {
    if (!Finite(settings.Gravity) || !std::isfinite(settings.FixedTimeStep) ||
        settings.FixedTimeStep <= 0.0f || settings.MaxSubSteps == 0) return false;
    m_Settings = settings;
    m_Accumulator = std::min(m_Accumulator, settings.FixedTimeStep);
    return true;
}
std::uint32_t PhysicsWorld3D::Advance(float frameDelta, std::vector<RigidBody3D>& bodies) {
    if (!std::isfinite(frameDelta) || frameDelta <= 0.0f) return 0;
    m_Accumulator += std::min(frameDelta, m_Settings.FixedTimeStep * m_Settings.MaxSubSteps);
    std::uint32_t steps = 0;
    while (m_Accumulator >= m_Settings.FixedTimeStep && steps < m_Settings.MaxSubSteps) {
        Step(m_Settings.FixedTimeStep, bodies);
        m_Accumulator -= m_Settings.FixedTimeStep;
        ++steps;
    }
    return steps;
}
void PhysicsWorld3D::Step(float dt, std::vector<RigidBody3D>& bodies) {
    if (!std::isfinite(dt) || dt <= 0.0f) return;
    std::vector<AABB> bounds;
    bounds.reserve(bodies.size());
    for (auto& body : bodies) {
        if (!body.Enabled || !body.IsValid() || body.InverseMass() == 0.0f) {
            body.Force = glm::vec3(0.0f);
        } else {
            const glm::vec3 acceleration = body.Force * body.InverseMass() +
                (body.UseGravity ? m_Settings.Gravity : glm::vec3(0.0f));
            body.Velocity += acceleration * dt;
            body.Position += body.Velocity * dt;
            body.Force = glm::vec3(0.0f);
        }
        bounds.push_back(AABB{body.Position - body.HalfExtents, body.Position + body.HalfExtents});
    }
    Broadphase3D broadphase;
    const auto pairs = broadphase.ComputePairs(bounds);
    for (int iteration = 0; iteration < 6; ++iteration) {
        for (const auto& [ia, ib] : pairs) {
            auto& a = bodies[ia]; auto& b = bodies[ib];
            if (!a.Enabled || !b.Enabled || !a.IsValid() || !b.IsValid()) continue;
            const float invA = a.InverseMass(), invB = b.InverseMass(), invSum = invA + invB;
            if (invSum <= 0.0f) continue;
            Contact contact;
            if (!Overlap(bounds[ia], bounds[ib], &contact)) continue;
            glm::vec3 relative = b.Velocity - a.Velocity;
            const float normalVelocity = glm::dot(relative, contact.Normal);
            float normalImpulse = 0.0f;
            if (normalVelocity < 0.0f) {
                const float restitution = std::min(a.Restitution, b.Restitution);
                normalImpulse = -(1.0f + restitution) * normalVelocity / invSum;
                const glm::vec3 impulse = contact.Normal * normalImpulse;
                a.Velocity -= impulse * invA;
                b.Velocity += impulse * invB;
                relative = b.Velocity - a.Velocity;
                glm::vec3 tangent = relative - contact.Normal * glm::dot(relative, contact.Normal);
                const float tangentLength = glm::length(tangent);
                if (tangentLength > 1.0e-6f) {
                    tangent /= tangentLength;
                    float jt = -glm::dot(relative, tangent) / invSum;
                    const float friction = std::sqrt(a.Friction * b.Friction);
                    jt = std::clamp(jt, -normalImpulse * friction, normalImpulse * friction);
                    const glm::vec3 frictionImpulse = tangent * jt;
                    a.Velocity -= frictionImpulse * invA;
                    b.Velocity += frictionImpulse * invB;
                }
            }
            const float correction = std::max(contact.Penetration - 0.005f, 0.0f) * 0.2f / invSum;
            const glm::vec3 positionImpulse = contact.Normal * correction;
            a.Position -= positionImpulse * invA;
            b.Position += positionImpulse * invB;
            bounds[ia] = AABB{a.Position - a.HalfExtents, a.Position + a.HalfExtents};
            bounds[ib] = AABB{b.Position - b.HalfExtents, b.Position + b.HalfExtents};
        }
    }
}
float PhysicsWorld3D::GetInterpolationAlpha() const {
    return m_Settings.FixedTimeStep > 0.0f
        ? std::clamp(m_Accumulator / m_Settings.FixedTimeStep, 0.0f, 1.0f) : 0.0f;
}

} // namespace vyra::physics3d
