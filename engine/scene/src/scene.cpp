#include "vyra/scene/scene.hpp"
#include "vyra/core/log.hpp"
#include "vyra/scene/components.hpp"

#include <algorithm>

namespace vyra::scene {

    Scene::Scene() : m_Name("Untitled Scene") {}

    Scene::Scene(const std::string& name) : m_Name(name) {}

    Scene::~Scene() {
        StopRuntime();
        m_Registry.Clear();
    }

    template<typename Component>
    static void CopyComponent(vyra::ecs::Registry& dst, vyra::ecs::Registry& src, const std::unordered_map<UUID, vyra::ecs::EntityID>& entMap) {
        src.Each<Component>([&](vyra::ecs::EntityID srcEntity, Component& component) {
            if (src.Has<TagComponent>(srcEntity)) {
                UUID uuid = src.Get<TagComponent>(srcEntity).ID;
                if (entMap.find(uuid) != entMap.end()) {
                    vyra::ecs::EntityID dstEntity = entMap.at(uuid);
                    dst.EmplaceOrReplace<Component>(dstEntity, component);
                }
            }
        });
    }

    Ref<Scene> Scene::Copy(Ref<Scene> other) {
        Ref<Scene> newScene = CreateRef<Scene>(other->m_Name);
        newScene->m_ViewportWidth = other->m_ViewportWidth;
        newScene->m_ViewportHeight = other->m_ViewportHeight;

        std::unordered_map<UUID, vyra::ecs::EntityID> entMap;

        // First pass: Create entities with matching UUIDs and TagComponent
        other->m_Registry.Each<TagComponent>([&](vyra::ecs::EntityID entityID, TagComponent& tag) {
            vyra::ecs::Entity newEntity = newScene->CreateEntityWithUUID(tag.ID, tag.Tag);
            entMap[tag.ID] = newEntity.GetID();
        });

        // Second pass: Copy components
        CopyComponent<TransformComponent>(newScene->m_Registry, other->m_Registry, entMap);
        CopyComponent<CameraComponent>(newScene->m_Registry, other->m_Registry, entMap);
        CopyComponent<MeshComponent>(newScene->m_Registry, other->m_Registry, entMap);
        CopyComponent<SpriteRendererComponent>(newScene->m_Registry, other->m_Registry, entMap);
        CopyComponent<DirectionalLightComponent>(newScene->m_Registry, other->m_Registry, entMap);
        CopyComponent<PointLightComponent>(newScene->m_Registry, other->m_Registry, entMap);

        return newScene;
    }

    vyra::ecs::Entity Scene::CreateEntity(const std::string& name) {
        return CreateEntityWithUUID(UUID(), name);
    }

    vyra::ecs::Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name) {
        vyra::ecs::EntityID id = m_Registry.Create();
        vyra::ecs::Entity entity(id, &m_Registry);

        m_Registry.Emplace<TagComponent>(id, name.empty() ? "Entity" : name, uuid);
        m_Registry.Emplace<TransformComponent>(id);
        return entity;
    }

    void Scene::DestroyEntity(vyra::ecs::Entity entity) {
        if (entity) {
            m_Registry.Destroy(entity.GetID());
        }
    }

    void Scene::Clear() {
        m_Registry.Clear();
        VYRA_LOG_INFO("Cleared scene '{0}'", m_Name);
    }

    bool Scene::AddRuntimeSystem(std::shared_ptr<SceneRuntimeSystem> system) {
        if (!system) return false;
        for (const auto& existing : m_RuntimeSystems) if (existing == system) return false;
        m_RuntimeSystems.push_back(system);
        if (m_RuntimeStarted) system->OnStart(*this);
        return true;
    }

    bool Scene::RemoveRuntimeSystem(const std::shared_ptr<SceneRuntimeSystem>& system) {
        const auto it = std::find(m_RuntimeSystems.begin(), m_RuntimeSystems.end(), system);
        if (it == m_RuntimeSystems.end()) return false;
        if (m_RuntimeStarted) (*it)->OnStop(*this);
        m_RuntimeSystems.erase(it);
        return true;
    }

    void Scene::StopRuntime() {
        if (!m_RuntimeStarted) return;
        for (auto it = m_RuntimeSystems.rbegin(); it != m_RuntimeSystems.rend(); ++it)
            (*it)->OnStop(*this);
        m_RuntimeStarted = false;
        m_RuntimeClock.Reset();
    }

    void Scene::OnUpdateRuntime(Timestep ts) {
        if (!m_RuntimeStarted) {
            m_RuntimeStarted = true;
            const auto systems = m_RuntimeSystems;
            for (const auto& system : systems) system->OnStart(*this);
        }

        const auto systems = m_RuntimeSystems;
        m_RuntimeClock.Advance(ts.GetSeconds(), [&](double fixedDelta) {
            for (const auto& system : systems)
                if (std::find(m_RuntimeSystems.begin(), m_RuntimeSystems.end(), system) != m_RuntimeSystems.end())
                    system->OnFixedUpdate(*this, Timestep(static_cast<float>(fixedDelta)));
        });
        for (const auto& system : systems)
            if (std::find(m_RuntimeSystems.begin(), m_RuntimeSystems.end(), system) != m_RuntimeSystems.end())
                system->OnUpdate(*this, ts);
    }

    void Scene::OnUpdateEditor(Timestep ts) {
        (void)ts;
        // Editor system updates
    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height) {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        m_Registry.Each<CameraComponent>([&](vyra::ecs::EntityID entity, CameraComponent& cameraComponent) {
            if (!cameraComponent.FixedAspectRatio) {
                cameraComponent.Camera.SetViewportSize(width, height);
            }
        });
    }

    vyra::ecs::Entity Scene::GetPrimaryCameraEntity() {
        vyra::ecs::Entity primaryCam;
        m_Registry.Each<CameraComponent>([&](vyra::ecs::EntityID entityID, CameraComponent& cameraComponent) {
            if (cameraComponent.Primary && !primaryCam) {
                primaryCam = vyra::ecs::Entity(entityID, &m_Registry);
            }
        });
        return primaryCam;
    }

} // namespace vyra::scene
