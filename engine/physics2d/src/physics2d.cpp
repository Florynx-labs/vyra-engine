#include "vyra/physics2d/physics2d.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace vyra::physics2d {
namespace {

bool IsFinite(const glm::vec2& value) {
    return std::isfinite(value.x) && std::isfinite(value.y);
}

glm::vec2 SafeNormal(const glm::vec2& value, const glm::vec2& fallback) {
    const float lengthSquared = glm::dot(value, value);
    if (lengthSquared <= std::numeric_limits<float>::epsilon()) {
        return fallback;
    }
    return value / std::sqrt(lengthSquared);
}

} // namespace

bool AABB::IsValid() const {
    return IsFinite(Min) && IsFinite(Max) && Min.x <= Max.x && Min.y <= Max.y;
}

glm::vec2 AABB::Center() const {
    return (Min + Max) * 0.5f;
}

glm::vec2 AABB::HalfExtents() const {
    return (Max - Min) * 0.5f;
}

bool Circle::IsValid() const {
    return IsFinite(Center) && std::isfinite(Radius) && Radius >= 0.0f;
}

bool Ray2D::IsValid() const {
    const float lengthSquared = glm::dot(Direction, Direction);
    return IsFinite(Origin) && IsFinite(Direction) &&
        std::isfinite(MaxDistance) && MaxDistance >= 0.0f &&
        std::abs(lengthSquared - 1.0f) <= 1.0e-3f;
}

bool Overlap(const AABB& first, const AABB& second, Contact* contact) {
    if (!first.IsValid() || !second.IsValid()) return false;

    const glm::vec2 delta = second.Center() - first.Center();
    const glm::vec2 overlap = first.HalfExtents() + second.HalfExtents() -
        glm::abs(delta);
    if (overlap.x < 0.0f || overlap.y < 0.0f) return false;

    if (contact) {
        const bool useX = overlap.x < overlap.y;
        contact->Normal = useX
            ? glm::vec2(delta.x < 0.0f ? -1.0f : 1.0f, 0.0f)
            : glm::vec2(0.0f, delta.y < 0.0f ? -1.0f : 1.0f);
        contact->Penetration = useX ? overlap.x : overlap.y;
        contact->Point = (glm::max(first.Min, second.Min) +
            glm::min(first.Max, second.Max)) * 0.5f;
    }
    return true;
}

bool Overlap(const Circle& first, const Circle& second, Contact* contact) {
    if (!first.IsValid() || !second.IsValid()) return false;

    const glm::vec2 delta = second.Center - first.Center;
    const float distanceSquared = glm::dot(delta, delta);
    const float radiusSum = first.Radius + second.Radius;
    if (distanceSquared > radiusSum * radiusSum) return false;

    if (contact) {
        const float distance = std::sqrt(std::max(distanceSquared, 0.0f));
        contact->Normal = SafeNormal(delta, glm::vec2(1.0f, 0.0f));
        contact->Penetration = radiusSum - distance;
        contact->Point = first.Center + contact->Normal *
            (first.Radius - contact->Penetration * 0.5f);
    }
    return true;
}

bool Overlap(const Circle& circle, const AABB& box, Contact* contact) {
    if (!circle.IsValid() || !box.IsValid()) return false;

    const glm::vec2 closest = glm::clamp(circle.Center, box.Min, box.Max);
    const glm::vec2 delta = closest - circle.Center;
    const float distanceSquared = glm::dot(delta, delta);
    if (distanceSquared > circle.Radius * circle.Radius) return false;

    if (contact) {
        const float distance = std::sqrt(std::max(distanceSquared, 0.0f));
        if (distance > std::numeric_limits<float>::epsilon()) {
            contact->Normal = delta / distance;
            contact->Penetration = circle.Radius - distance;
            contact->Point = closest;
        } else {
            // The circle center is inside the box: choose the nearest face.
            const glm::vec2 toMin = circle.Center - box.Min;
            const glm::vec2 toMax = box.Max - circle.Center;
            const float left = toMin.x;
            const float right = toMax.x;
            const float bottom = toMin.y;
            const float top = toMax.y;
            const float nearest = std::min({left, right, bottom, top});
            if (nearest == left) contact->Normal = {-1.0f, 0.0f};
            else if (nearest == right) contact->Normal = {1.0f, 0.0f};
            else if (nearest == bottom) contact->Normal = {0.0f, -1.0f};
            else contact->Normal = {0.0f, 1.0f};
            contact->Penetration = circle.Radius + nearest;
            contact->Point = circle.Center + contact->Normal * nearest;
        }
    }
    return true;
}

bool Overlap(const AABB& box, const Circle& circle, Contact* contact) {
    const bool hit = Overlap(circle, box, contact);
    if (hit && contact) contact->Normal = -contact->Normal;
    return hit;
}

std::optional<RayHit> Raycast(const Ray2D& ray, const AABB& box) {
    if (!ray.IsValid() || !box.IsValid()) return std::nullopt;

    float nearDistance = 0.0f;
    float farDistance = ray.MaxDistance;
    glm::vec2 nearNormal{0.0f};

    for (int axis = 0; axis < 2; ++axis) {
        const float origin = ray.Origin[axis];
        const float direction = ray.Direction[axis];
        if (std::abs(direction) <= std::numeric_limits<float>::epsilon()) {
            if (origin < box.Min[axis] || origin > box.Max[axis]) return std::nullopt;
            continue;
        }

        float first = (box.Min[axis] - origin) / direction;
        float second = (box.Max[axis] - origin) / direction;
        float sign = -1.0f;
        if (first > second) {
            std::swap(first, second);
            sign = 1.0f;
        }
        if (first > nearDistance) {
            nearDistance = first;
            nearNormal = glm::vec2(0.0f);
            nearNormal[axis] = sign;
        }
        farDistance = std::min(farDistance, second);
        if (nearDistance > farDistance) return std::nullopt;
    }

    if (farDistance < 0.0f || nearDistance > ray.MaxDistance) return std::nullopt;
    const float distance = std::max(nearDistance, 0.0f);
    return RayHit{ray.Origin + ray.Direction * distance, nearNormal, distance};
}

std::optional<RayHit> Raycast(const Ray2D& ray, const Circle& circle) {
    if (!ray.IsValid() || !circle.IsValid()) return std::nullopt;

    const glm::vec2 offset = ray.Origin - circle.Center;
    const float b = glm::dot(offset, ray.Direction);
    const float c = glm::dot(offset, offset) - circle.Radius * circle.Radius;
    const float discriminant = b * b - c;
    if (discriminant < 0.0f) return std::nullopt;

    const float root = std::sqrt(discriminant);
    float distance = -b - root;
    if (distance < 0.0f) distance = -b + root;
    if (distance < 0.0f || distance > ray.MaxDistance) return std::nullopt;

    const glm::vec2 point = ray.Origin + ray.Direction * distance;
    return RayHit{point, SafeNormal(point - circle.Center, -ray.Direction), distance};
}

} // namespace vyra::physics2d
