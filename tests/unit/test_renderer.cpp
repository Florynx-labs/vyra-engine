#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "vyra/scene/editor_camera.hpp"
#include "vyra/renderer/mesh.hpp"
#include "vyra/renderer/mesh_renderer.hpp"

using namespace vyra;

TEST_CASE("VYRA Scene - EditorCamera Initial Projections & Orbit", "[scene][camera]") {
    scene::EditorCamera camera(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

    REQUIRE(camera.GetFOV() > 0.0f);
    REQUIRE(camera.GetNearClip() == 0.1f);
    REQUIRE(camera.GetFarClip() == 1000.0f);

    glm::mat4 proj = camera.GetProjectionMatrix();
    // Vulkan Y-flip test
    REQUIRE(proj[1][1] < 0.0f);

    // Orbit update test
    scene::CameraInput input;
    input.RightMouseDown = true;
    input.MouseDeltaX = 10.0f;
    input.MouseDeltaY = 5.0f;

    float initialYaw = camera.GetYaw();
    camera.OnUpdate(input, 0.016f);
    REQUIRE(camera.GetYaw() != initialYaw);

    glm::mat4 viewProj = camera.GetViewProjection();
    REQUIRE(viewProj != glm::mat4(1.0f));
}

TEST_CASE("VYRA Renderer - Procedural Primitive Mesh Generation", "[renderer][mesh]") {
    SECTION("Cube Generation") {
        auto cube = renderer::Mesh::CreateCube(1.0f);
        REQUIRE(cube.Name == "Cube");
        REQUIRE(cube.Vertices.size() == 24); // 6 faces * 4 vertices
        REQUIRE(cube.Indices.size() == 36);  // 6 faces * 6 indices (2 tris)
    }

    SECTION("Grid Floor Generation") {
        auto grid = renderer::Mesh::CreateGrid(10, 1.0f);
        REQUIRE(grid.Name == "Grid");
        REQUIRE(grid.Vertices.size() > 0);
        REQUIRE(grid.Indices.size() > 0);
    }

    SECTION("Sphere Generation") {
        auto sphere = renderer::Mesh::CreateSphere(1.0f, 8, 8);
        REQUIRE(sphere.Name == "Sphere");
        REQUIRE(sphere.Vertices.size() == 81); // (rings+1)*(sectors+1) = 9*9
        REQUIRE(sphere.Indices.size() == 8 * 8 * 6); // 8*8 quads * 6 indices
    }
}
