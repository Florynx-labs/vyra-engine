#include "vyra/physics2d/world.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cmath>

using namespace vyra::physics2d;

TEST_CASE("Spatial hash returns unique deterministic candidate pairs", "[physics2d][broadphase]") {
    SpatialHash2D grid(1.0f);
    grid.Rebuild({
        AABB{{0.0f, 0.0f}, {2.0f, 2.0f}},
        AABB{{1.0f, 1.0f}, {3.0f, 3.0f}},
        AABB{{20.0f, 20.0f}, {21.0f, 21.0f}}
    });
    const auto pairs = grid.ComputePairs();
    REQUIRE(pairs.size() == 1);
    REQUIRE(pairs[0].first == 0);
    REQUIRE(pairs[0].second == 1);
}

TEST_CASE("Spatial hash ignores invalid and excessively large bounds", "[physics2d][broadphase]") {
    SpatialHash2D grid(1.0f, 4);
    grid.Rebuild({
        AABB{{0.0f, 0.0f}, {1.0f, 1.0f}},
        AABB{{0.0f, 0.0f}, {100.0f, 100.0f}},
        AABB{{2.0f, 0.0f}, {1.0f, 1.0f}}
    });
    REQUIRE(grid.ComputePairs().empty());
}

TEST_CASE("Physics world advances bodies at a fixed timestep", "[physics2d][world]") {
    PhysicsWorldSettings2D settings;
    settings.Gravity = {0.0f, -10.0f};
    settings.FixedTimeStep = 0.1f;
    settings.MaxSubSteps = 4;
    PhysicsWorld2D world(settings);

    std::vector<RigidBody2D> bodies(1);
    REQUIRE(world.Advance(0.1f, bodies) == 1);
    REQUIRE(std::abs(bodies[0].Velocity.y + 1.0f) < 1.0e-4f);
    REQUIRE(std::abs(bodies[0].Position.y + 0.1f) < 1.0e-4f);
}

TEST_CASE("Static bodies do not move and forces are consumed once", "[physics2d][world]") {
    PhysicsWorld2D world;
    std::vector<RigidBody2D> bodies(2);
    bodies[0].Mass = 0.0f;
    bodies[0].ApplyForce({100.0f, 0.0f});
    bodies[1].UseGravity = false;
    bodies[1].ApplyForce({10.0f, 0.0f});

    world.Step(0.5f, bodies);
    REQUIRE(bodies[0].Position == glm::vec2(0.0f));
    REQUIRE(std::abs(bodies[1].Velocity.x - 5.0f) < 1.0e-4f);
    REQUIRE(std::abs(bodies[1].Position.x - 2.5f) < 1.0e-4f);
    REQUIRE(bodies[1].Force == glm::vec2(0.0f));
}

TEST_CASE("Physics world rejects invalid settings and bounds substeps", "[physics2d][world]") {
    PhysicsWorld2D world;
    auto invalid = world.GetSettings();
    invalid.FixedTimeStep = 0.0f;
    REQUIRE_FALSE(world.SetSettings(invalid));

    auto settings = world.GetSettings();
    settings.FixedTimeStep = 0.01f;
    settings.MaxSubSteps = 2;
    REQUIRE(world.SetSettings(settings));
    std::vector<RigidBody2D> bodies(1);
    REQUIRE(world.Advance(1.0f, bodies) == 2);
    REQUIRE(world.GetInterpolationAlpha() >= 0.0f);
    REQUIRE(world.GetInterpolationAlpha() <= 1.0f);
}
