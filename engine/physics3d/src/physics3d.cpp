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
} // namespace vyra::physics3d
