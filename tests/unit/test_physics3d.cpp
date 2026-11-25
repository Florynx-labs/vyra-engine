#include "vyra/physics3d/physics3d.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cmath>

using namespace vyra::physics3d;

TEST_CASE("3D AABB overlap produces a unit contact normal", "[physics3d]") {
    const AABB a{{0.0f}, {2.0f}};
    const AABB b{{1.5f, 0.5f, 0.5f}, {3.0f, 1.5f, 1.5f}};
    Contact contact{};
    REQUIRE(Overlap(a, b, &contact));
    REQUIRE(std::abs(contact.Penetration - 0.5f) < 1.0e-4f);
    REQUIRE(std::abs(glm::length(contact.Normal) - 1.0f) < 1.0e-4f);
}

TEST_CASE("3D spheres detect tangency", "[physics3d]") {
    const Sphere a{{0.0f}, 1.0f};
    const Sphere b{{2.0f, 0.0f, 0.0f}, 1.0f};
    Contact contact{};
    REQUIRE(Overlap(a, b, &contact));
    REQUIRE(std::abs(contact.Penetration) < 1.0e-4f);
}

TEST_CASE("3D sphere detects a box and containment", "[physics3d]") {
    const Sphere sphere{{1.0f}, 0.25f};
    const AABB box{{0.0f}, {4.0f}};
    Contact contact{};
    REQUIRE(Overlap(sphere, box, &contact));
    REQUIRE(contact.Penetration > 0.25f);
}

TEST_CASE("3D raycasts return nearest hit and respect range", "[physics3d]") {
    const AABB box{{0.0f}, {1.0f}};
    const Ray3D ray{{-2.0f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, 10.0f};
    const auto hit = Raycast(ray, box);
    REQUIRE(hit.has_value());
    REQUIRE(std::abs(hit->Distance - 2.0f) < 1.0e-4f);

    const Ray3D shortRay{{-2.0f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, 1.0f};
    REQUIRE_FALSE(Raycast(shortRay, box).has_value());
}

TEST_CASE("3D raycast supports spheres", "[physics3d]") {
    const Sphere sphere{{0.0f}, 1.0f};
    const Ray3D ray{{-3.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, 10.0f};
    const auto hit = Raycast(ray, sphere);
    REQUIRE(hit.has_value());
    REQUIRE(std::abs(hit->Distance - 2.0f) < 1.0e-4f);
}

TEST_CASE("3D shapes reject invalid dimensions", "[physics3d]") {
    REQUIRE_FALSE((AABB{{2.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}).IsValid());
    REQUIRE_FALSE((Sphere{{0.0f}, -1.0f}).IsValid());
    REQUIRE_FALSE((Ray3D{{0.0f}, {2.0f, 0.0f, 0.0f}, 10.0f}).IsValid());
}
