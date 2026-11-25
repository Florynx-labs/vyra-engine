#include "vyra/physics2d/physics2d.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cmath>

using namespace vyra::physics2d;

TEST_CASE("2D AABB overlap reports a stable contact", "[physics2d]") {
    const AABB first{{0.0f, 0.0f}, {2.0f, 2.0f}};
    const AABB second{{1.5f, 0.5f}, {3.0f, 1.5f}};
    Contact contact{};

    REQUIRE(Overlap(first, second, &contact));
    REQUIRE(std::abs(contact.Penetration - 0.5f) < 1.0e-4f);
    REQUIRE(std::abs(contact.Normal.x - 1.0f) < 1.0e-4f);
    REQUIRE(std::abs(contact.Normal.y) < 1.0e-4f);
}

TEST_CASE("2D circle overlap handles tangency", "[physics2d]") {
    const Circle first{{0.0f, 0.0f}, 1.0f};
    const Circle second{{2.0f, 0.0f}, 1.0f};
    Contact contact{};

    REQUIRE(Overlap(first, second, &contact));
    REQUIRE(std::abs(contact.Penetration) < 1.0e-4f);
}

TEST_CASE("2D circle and box overlap detects containment", "[physics2d]") {
    const Circle circle{{1.0f, 1.0f}, 0.25f};
    const AABB box{{0.0f, 0.0f}, {4.0f, 4.0f}};
    Contact contact{};

    REQUIRE(Overlap(circle, box, &contact));
    REQUIRE(contact.Penetration > 0.25f);
    REQUIRE(std::abs(glm::length(contact.Normal) - 1.0f) < 1.0e-4f);
}

TEST_CASE("2D raycast returns nearest AABB hit", "[physics2d]") {
    const Ray2D ray{{-2.0f, 0.5f}, {1.0f, 0.0f}, 10.0f};
    const AABB box{{0.0f, 0.0f}, {1.0f, 1.0f}};

    const auto hit = Raycast(ray, box);
    REQUIRE(hit.has_value());
    REQUIRE(std::abs(hit->Distance - 2.0f) < 1.0e-4f);
    REQUIRE(std::abs(hit->Normal.x + 1.0f) < 1.0e-4f);
}

TEST_CASE("2D raycast rejects hits beyond its maximum distance", "[physics2d]") {
    const Ray2D ray{{-2.0f, 0.5f}, {1.0f, 0.0f}, 1.0f};
    const AABB box{{0.0f, 0.0f}, {1.0f, 1.0f}};

    REQUIRE_FALSE(Raycast(ray, box).has_value());
}

TEST_CASE("2D colliders reject invalid dimensions", "[physics2d]") {
    REQUIRE_FALSE((AABB{{2.0f, 0.0f}, {1.0f, 1.0f}}).IsValid());
    REQUIRE_FALSE((Circle{{0.0f, 0.0f}, -1.0f}).IsValid());
    REQUIRE_FALSE((Ray2D{{0.0f, 0.0f}, {2.0f, 0.0f}, 10.0f}).IsValid());
}
