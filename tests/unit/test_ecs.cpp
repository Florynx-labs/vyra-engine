#include <catch2/catch_test_macros.hpp>
#include <vyra/ecs/ecs.hpp>

TEST_CASE("VYRA ECS - Entity Creation and Destruction", "[ecs]") {
    vyra::ecs::Registry registry;
    REQUIRE(registry.Size() == 0);

    vyra::ecs::EntityID e1 = registry.Create();
    vyra::ecs::EntityID e2 = registry.Create();
    REQUIRE(registry.Size() == 2);

    registry.Destroy(e1);
    REQUIRE(registry.Size() == 1);

    registry.Clear();
    REQUIRE(registry.Size() == 0);
}

TEST_CASE("VYRA ECS - Component Emplace & Query", "[ecs]") {
    vyra::ecs::Registry registry;
    vyra::ecs::EntityID e1 = registry.Create();

    registry.Emplace<vyra::ecs::TagComponent>(e1, "PlayerEntity");
    registry.Emplace<vyra::ecs::TransformComponent>(e1, glm::vec3(10.0f, 5.0f, 0.0f));

    REQUIRE(registry.Has<vyra::ecs::TagComponent>(e1));
    REQUIRE(registry.Has<vyra::ecs::TransformComponent>(e1));

    const auto& tag = registry.Get<vyra::ecs::TagComponent>(e1);
    REQUIRE(tag.Tag == "PlayerEntity");

    auto& transform = registry.Get<vyra::ecs::TransformComponent>(e1);
    REQUIRE(transform.Translation.x == 10.0f);
    REQUIRE(transform.Translation.y == 5.0f);
}

TEST_CASE("VYRA ECS - Transform Matrix Calculation", "[ecs]") {
    vyra::ecs::TransformComponent transform;
    transform.Translation = glm::vec3(2.0f, 3.0f, 4.0f);
    transform.Scale = glm::vec3(2.0f, 2.0f, 2.0f);

    glm::mat4 mat = transform.GetTransform();
    REQUIRE(mat[3][0] == 2.0f);
    REQUIRE(mat[3][1] == 3.0f);
    REQUIRE(mat[3][2] == 4.0f);
}

TEST_CASE("VYRA ECS - Entity Handle Wrapper", "[ecs]") {
    vyra::ecs::Registry registry;
    vyra::ecs::Entity entity(registry.Create(), &registry);

    entity.AddComponent<vyra::ecs::TagComponent>("EnemyEntity");
    entity.AddComponent<vyra::ecs::TransformComponent>(glm::vec3(0.0f, 1.0f, 0.0f));

    REQUIRE(entity.HasComponent<vyra::ecs::TagComponent>());
    REQUIRE(entity.GetName() == "EnemyEntity");
    REQUIRE(entity.GetComponent<vyra::ecs::TransformComponent>().Translation.y == 1.0f);

    entity.RemoveComponent<vyra::ecs::TransformComponent>();
    REQUIRE_FALSE(entity.HasComponent<vyra::ecs::TransformComponent>());
}

TEST_CASE("VYRA ECS - Registry Each View Iteration", "[ecs]") {
    vyra::ecs::Registry registry;
    for (int i = 0; i < 5; ++i) {
        auto id = registry.Create();
        registry.Emplace<vyra::ecs::TagComponent>(id, "Entity_" + std::to_string(i));
        registry.Emplace<vyra::ecs::TransformComponent>(id, glm::vec3((float)i, 0.0f, 0.0f));
    }

    size_t count = 0;
    registry.Each<vyra::ecs::TagComponent, vyra::ecs::TransformComponent>([&count](vyra::ecs::EntityID id, vyra::ecs::TagComponent& tag, vyra::ecs::TransformComponent& transform) {
        count++;
        REQUIRE(transform.Translation.x >= 0.0f);
    });

    REQUIRE(count == 5);
}
