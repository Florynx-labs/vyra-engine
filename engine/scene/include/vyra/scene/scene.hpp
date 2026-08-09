#pragma once

#include "vyra/core/base.hpp"
#include "vyra/core/time.hpp"
#include "vyra/core/uuid.hpp"
#include "vyra/ecs/ecs.hpp"
#include "vyra/scene/components.hpp"

#include <string>

namespace vyra::scene {

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

        friend class vyra::ecs::Entity;
        friend class SceneSerializer;
    };

} // namespace vyra::scene
