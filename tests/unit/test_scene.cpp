#include <catch2/catch_test_macros.hpp>
#include "vyra/scene/scene.hpp"
#include "vyra/scene/scene_serializer.hpp"
#include "vyra/scene/components.hpp"

#include <filesystem>

TEST_CASE("VYRA Scene - Entity Creation & Component Attachment", "[scene]") {
    vyra::Ref<vyra::scene::Scene> scene = vyra::CreateRef<vyra::scene::Scene>("Test Scene");
    REQUIRE(scene->GetName() == "Test Scene");

    vyra::ecs::Entity player = scene->CreateEntity("Player");
    REQUIRE(player);
    REQUIRE(player.GetName() == "Player");
    REQUIRE(player.HasComponent<vyra::scene::TagComponent>());
    REQUIRE(player.HasComponent<vyra::scene::TransformComponent>());

    auto& mesh = player.AddComponent<vyra::scene::MeshComponent>("assets/models/cube.gltf", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    REQUIRE(player.HasComponent<vyra::scene::MeshComponent>());
    REQUIRE(player.GetComponent<vyra::scene::MeshComponent>().AssetPath == "assets/models/cube.gltf");
    REQUIRE(mesh.Color.r == 1.0f);

    REQUIRE(scene->GetRegistry().Size() == 1);

    scene->DestroyEntity(player);
    REQUIRE(scene->GetRegistry().Size() == 0);
}

TEST_CASE("VYRA Scene - Primary Camera Query & Aspect Ratio Update", "[scene]") {
    vyra::Ref<vyra::scene::Scene> scene = vyra::CreateRef<vyra::scene::Scene>("Camera Scene");

    vyra::ecs::Entity cameraEntity = scene->CreateEntity("Main Camera");
    auto& cameraComp = cameraEntity.AddComponent<vyra::scene::CameraComponent>();
    cameraComp.Primary = true;

    vyra::ecs::Entity foundCam = scene->GetPrimaryCameraEntity();
    REQUIRE(foundCam);
    REQUIRE(foundCam.GetID() == cameraEntity.GetID());
    REQUIRE(foundCam.GetName() == "Main Camera");

    scene->OnViewportResize(1920, 1080);
    // 1920/1080 ~ 1.777778
    const auto& proj = cameraComp.Camera.GetProjection();
    REQUIRE(proj[0][0] > 0.0f);
}

TEST_CASE("VYRA Scene - Scene Cloning (Play Mode Copy)", "[scene]") {
    vyra::Ref<vyra::scene::Scene> editorScene = vyra::CreateRef<vyra::scene::Scene>("Editor World");

    vyra::ecs::Entity entity1 = editorScene->CreateEntity("Cube");
    auto& t1 = entity1.GetComponent<vyra::scene::TransformComponent>();
    t1.Translation = glm::vec3(5.0f, 2.0f, -10.0f);

    entity1.AddComponent<vyra::scene::MeshComponent>("models/mesh.obj", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

    vyra::ecs::Entity entity2 = editorScene->CreateEntity("Light");
    entity2.AddComponent<vyra::scene::DirectionalLightComponent>();

    REQUIRE(editorScene->GetRegistry().Size() == 2);

    // Deep clone scene (e.g. entering play mode)
    vyra::Ref<vyra::scene::Scene> runtimeScene = vyra::scene::Scene::Copy(editorScene);
    REQUIRE(runtimeScene);
    REQUIRE(runtimeScene->GetName() == "Editor World");
    REQUIRE(runtimeScene->GetRegistry().Size() == 2);

    // Modify runtime entity — editor entity should remain unchanged
    runtimeScene->GetRegistry().Each<vyra::scene::TransformComponent>([&](vyra::ecs::EntityID, vyra::scene::TransformComponent& transform) {
        transform.Translation.x += 10.0f;
    });

    REQUIRE(t1.Translation.x == 5.0f);
}

TEST_CASE("VYRA Scene - Deterministic JSON Serialization Roundtrip", "[scene]") {
    std::string tempFilepath = "test_scene_save.vyra";

    // Setup scene to serialize
    {
        vyra::Ref<vyra::scene::Scene> srcScene = vyra::CreateRef<vyra::scene::Scene>("Serialization Test");

        vyra::ecs::Entity cam = srcScene->CreateEntity("MainCam");
        auto& cc = cam.AddComponent<vyra::scene::CameraComponent>();
        cc.Primary = true;

        vyra::ecs::Entity obj = srcScene->CreateEntity("RedCube");
        auto& tc = obj.GetComponent<vyra::scene::TransformComponent>();
        tc.Translation = glm::vec3(1.0f, 2.0f, 3.0f);
        tc.Rotation = glm::vec3(0.0f, 0.5f, 0.0f);
        tc.Scale = glm::vec3(2.0f, 2.0f, 2.0f);

        obj.AddComponent<vyra::scene::MeshComponent>("assets/cube.fbx", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        obj.AddComponent<vyra::scene::PointLightComponent>();

        vyra::scene::SceneSerializer serializer(srcScene);
        serializer.Serialize(tempFilepath);

        REQUIRE(std::filesystem::exists(tempFilepath));
    }

    // Deserialize into fresh scene and verify data integrity
    {
        vyra::Ref<vyra::scene::Scene> dstScene = vyra::CreateRef<vyra::scene::Scene>();
        vyra::scene::SceneSerializer deserializer(dstScene);
        bool success = deserializer.Deserialize(tempFilepath);
        REQUIRE(success);

        REQUIRE(dstScene->GetName() == "Serialization Test");
        REQUIRE(dstScene->GetRegistry().Size() == 2);

        vyra::ecs::Entity cam = dstScene->GetPrimaryCameraEntity();
        REQUIRE(cam);
        REQUIRE(cam.GetName() == "MainCam");

        bool foundCube = false;
        dstScene->GetRegistry().Each<vyra::scene::MeshComponent>([&](vyra::ecs::EntityID id, vyra::scene::MeshComponent& mesh) {
            vyra::ecs::Entity entity(id, &dstScene->GetRegistry());
            if (entity.GetName() == "RedCube") {
                foundCube = true;
                REQUIRE(mesh.AssetPath == "assets/cube.fbx");
                REQUIRE(mesh.Color.r == 1.0f);

                auto& tc = entity.GetComponent<vyra::scene::TransformComponent>();
                REQUIRE(tc.Translation == glm::vec3(1.0f, 2.0f, 3.0f));
                REQUIRE(tc.Scale == glm::vec3(2.0f, 2.0f, 2.0f));

                REQUIRE(entity.HasComponent<vyra::scene::PointLightComponent>());
            }
        });

        REQUIRE(foundCube);
    }

    // Clean up temp file
    if (std::filesystem::exists(tempFilepath)) {
        std::filesystem::remove(tempFilepath);
    }
}

TEST_CASE("VYRA Scene - Play Mode World Isolation & Entity Lifecycle", "[scene][playmode]") {
    vyra::Ref<vyra::scene::Scene> editorScene = vyra::CreateRef<vyra::scene::Scene>("Editor World");

    vyra::ecs::Entity staticObj = editorScene->CreateEntity("StaticObject");
    auto& staticTransform = staticObj.GetComponent<vyra::scene::TransformComponent>();
    staticTransform.Translation = glm::vec3(1.0f, 0.0f, 0.0f);

    vyra::ecs::Entity dynamicObj = editorScene->CreateEntity("DynamicObject");
    auto& dynamicTransform = dynamicObj.GetComponent<vyra::scene::TransformComponent>();
    dynamicTransform.Translation = glm::vec3(0.0f, 5.0f, 0.0f);

    REQUIRE(editorScene->GetRegistry().Size() == 2);

    // --- Start Play Mode ---
    vyra::Ref<vyra::scene::Scene> runtimeScene = vyra::scene::Scene::Copy(editorScene);
    REQUIRE(runtimeScene->GetRegistry().Size() == 2);

    // 1. Mutate transform during play mode
    runtimeScene->GetRegistry().Each<vyra::scene::TransformComponent>([&](vyra::ecs::EntityID id, vyra::scene::TransformComponent& tc) {
        vyra::ecs::Entity entity(id, &runtimeScene->GetRegistry());
        if (entity.GetName() == "DynamicObject") {
            tc.Translation.y = 100.0f; // Falling object / physics explosion
        }
    });

    // 2. Spawn temporary particle entity in play mode
    vyra::ecs::Entity spawnedParticle = runtimeScene->CreateEntity("SpawnedParticle");
    REQUIRE(runtimeScene->GetRegistry().Size() == 3);

    // 3. Destroy static object in play mode
    runtimeScene->GetRegistry().Each<vyra::scene::TagComponent>([&](vyra::ecs::EntityID id, vyra::scene::TagComponent& tag) {
        if (tag.Tag == "StaticObject") {
            runtimeScene->DestroyEntity(vyra::ecs::Entity(id, &runtimeScene->GetRegistry()));
        }
    });

    REQUIRE(runtimeScene->GetRegistry().Size() == 2); // DynamicObject + SpawnedParticle

    // --- Verify Editor World Isolation ---
    // The original editor scene must remain 100% untouched
    REQUIRE(editorScene->GetRegistry().Size() == 2);
    REQUIRE(staticTransform.Translation == glm::vec3(1.0f, 0.0f, 0.0f));
    REQUIRE(dynamicTransform.Translation == glm::vec3(0.0f, 5.0f, 0.0f));

    // --- Stop Play Mode ---
    // In EditorLayer, activeScene is simply reset to editorScene
    vyra::Ref<vyra::scene::Scene> restoredScene = editorScene;
    REQUIRE(restoredScene->GetRegistry().Size() == 2);
}

