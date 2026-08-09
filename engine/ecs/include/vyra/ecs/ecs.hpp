#pragma once

#include "vyra/core/base.hpp"
#include "vyra/core/uuid.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <entt/entt.hpp>

#include <cstdint>
#include <string>
#include <tuple>

namespace vyra::ecs {

    using EntityID = entt::entity;
    constexpr EntityID NullEntity = entt::null;

    class Registry;

    // --- Core ECS Components ---
    struct VYRA_API TagComponent {
        std::string Tag;
        UUID ID;

        TagComponent() = default;
        TagComponent(const std::string& tag) : Tag(tag), ID() {}
        TagComponent(const std::string& tag, UUID id) : Tag(tag), ID(id) {}
    };

    struct VYRA_API TransformComponent {
        glm::vec3 Translation{ 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation{ 0.0f, 0.0f, 0.0f }; // Euler angles in radians
        glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };

        TransformComponent() = default;
        TransformComponent(const glm::vec3& translation) : Translation(translation) {}

        glm::mat4 GetTransform() const {
            glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
            return glm::translate(glm::mat4(1.0f), Translation)
                * rotation
                * glm::scale(glm::mat4(1.0f), Scale);
        }
    };

    // --- Registry Wrapper ---
    class VYRA_API Registry {
    public:
        Registry() = default;
        ~Registry() = default;

        EntityID Create();
        void Destroy(EntityID entity);
        void Clear();

        template<typename T, typename... Args>
        T& Emplace(EntityID entity, Args&&... args) {
            return m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        T& Replace(EntityID entity, Args&&... args) {
            return m_Registry.replace<T>(entity, std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        T& EmplaceOrReplace(EntityID entity, Args&&... args) {
            return m_Registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
        }

        template<typename T>
        bool Has(EntityID entity) const {
            return m_Registry.all_of<T>(entity);
        }

        template<typename T>
        T& Get(EntityID entity) {
            return m_Registry.get<T>(entity);
        }

        template<typename T>
        const T& Get(EntityID entity) const {
            return m_Registry.get<T>(entity);
        }

        template<typename T>
        T* TryGet(EntityID entity) {
            return m_Registry.try_get<T>(entity);
        }

        template<typename T>
        void Remove(EntityID entity) {
            m_Registry.remove<T>(entity);
        }

        template<typename... Components, typename Func>
        void Each(Func&& func) {
            auto view = m_Registry.view<Components...>();
            for (auto entity : view) {
                if constexpr (sizeof...(Components) == 1) {
                    func(entity, view.template get<Components...>(entity));
                } else {
                    std::apply([&](auto&... comps) {
                        func(entity, comps...);
                    }, view.template get<Components...>(entity));
                }
            }
        }

        size_t Size() const {
            auto storage = m_Registry.storage<entt::entity>();
            return storage ? storage->in_use() : 0;
        }

        entt::registry& GetNativeRegistry() { return m_Registry; }
        const entt::registry& GetNativeRegistry() const { return m_Registry; }

    private:
        entt::registry m_Registry;
    };

    // --- Entity Handle Wrapper ---
    class VYRA_API Entity {
    public:
        Entity() = default;
        Entity(EntityID handle, Registry* registry)
            : m_EntityHandle(handle), m_Registry(registry) {}

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args) {
            return m_Registry->Emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
        }

        template<typename T>
        T& GetComponent() {
            return m_Registry->Get<T>(m_EntityHandle);
        }

        template<typename T>
        const T& GetComponent() const {
            return m_Registry->Get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent() const {
            return m_Registry->Has<T>(m_EntityHandle);
        }

        template<typename T>
        void RemoveComponent() {
            m_Registry->Remove<T>(m_EntityHandle);
        }

        bool operator==(const Entity& other) const {
            return m_EntityHandle == other.m_EntityHandle && m_Registry == other.m_Registry;
        }
        bool operator!=(const Entity& other) const {
            return !(*this == other);
        }

        operator bool() const {
            return m_EntityHandle != NullEntity && m_Registry != nullptr && m_Registry->GetNativeRegistry().valid(m_EntityHandle);
        }
        operator EntityID() const { return m_EntityHandle; }

        EntityID GetID() const { return m_EntityHandle; }
        UUID GetUUID() const { return GetComponent<TagComponent>().ID; }
        const std::string& GetName() const { return GetComponent<TagComponent>().Tag; }

    private:
        EntityID m_EntityHandle{ NullEntity };
        Registry* m_Registry{ nullptr };
    };

} // namespace vyra::ecs
