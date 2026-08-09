#include <catch2/catch_test_macros.hpp>
#include "vyra/scene/scene.hpp"
#include "vyra/scene/scene_serializer.hpp"

TEST_CASE("Minimal registry reset", "[debug]") {
    vyra::Ref<vyra::scene::Scene> scene = vyra::CreateRef<vyra::scene::Scene>("Debug");
    auto entity = scene->CreateEntity("A");
    entity.AddComponent<vyra::scene::MeshComponent>();
    scene->Clear();
    auto entity2 = scene->CreateEntity("B");
    entity2.AddComponent<vyra::scene::MeshComponent>();
}
