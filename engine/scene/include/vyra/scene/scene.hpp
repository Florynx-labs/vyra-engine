#pragma once

#include "vyra/core/base.hpp"
#include "vyra/core/time.hpp"
#include "vyra/core/uuid.hpp"
#include "vyra/ecs/ecs.hpp"
#include "vyra/scene/components.hpp"
#include "vyra/runtime/fixed_step_clock.hpp"

#include <memory>
#include <vector>

#include <string>

namespace vyra::scene {

    class Scene;

    class VYRA_API SceneRuntimeSystem {
    public:
        virtual ~SceneRuntimeSystem() = default;
        virtual void OnStart(Scene&) {}
        virtual void OnFixedUpdate(Scene&, Timestep) {}
        virtual void OnUpdate(Scene&, Timestep) {}
        virtual void OnStop(Scene&) {}
    };

    class VYRA_API Scene {
    public:
        Scene();
        Scene(const std::string& name);
        ~Scene();

        static Ref<Scene> Copy(Ref<Scene> other);

        vyra::ecs::Entity CreateEntity(const std::string& name = "Entity");
        vyra::ecs::Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity");
        void DestroyEntity(vyra::ecs::Entity entity);
        void Clear();

        void OnUpdateRuntime(Timestep ts);
        bool AddRuntimeSystem(std::shared_ptr<SceneRuntimeSystem> system);
        bool RemoveRuntimeSystem(const std::shared_ptr<SceneRuntimeSystem>& system);
        void StopRuntime();
        void OnUpdateEditor(Timestep ts);
        void OnViewportResize(uint32_t width, uint32_t height);

        vyra::ecs::Entity GetPrimaryCameraEntity();

        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

        vyra::ecs::Registry& GetRegistry() { return m_Registry; }
        const vyra::ecs::Registry& GetRegistry() const { return m_Registry; }

        template<typename... Components>
        auto GetAllEntitiesWith() {
            return m_Registry.GetNativeRegistry().view<Components...>();
        }

    private:
        template<typename T>
        void OnComponentAdded(vyra::ecs::Entity entity, T& component);

    private:
        std::string m_Name{ "Untitled Scene" };
        vyra::ecs::Registry m_Registry;
        uint32_t m_ViewportWidth{ 0 };
        uint32_t m_ViewportHeight{ 0 };
        std::vector<std::shared_ptr<SceneRuntimeSystem>> m_RuntimeSystems;
        vyra::runtime::FixedStepClock m_RuntimeClock{1.0 / 60.0, 8, 0.25};
        bool m_RuntimeStarted{ false };

        friend class vyra::ecs::Entity;
        friend class SceneSerializer;
    };

} // namespace vyra::scene
