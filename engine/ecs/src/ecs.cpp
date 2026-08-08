#include "vyra/ecs/ecs.hpp"

namespace vyra::ecs {

    EntityID Registry::Create() {
        entt::entity entity = m_Registry.create();
        return static_cast<EntityID>(entity);
    }

    void Registry::Destroy(EntityID entity) {
        m_Registry.destroy(static_cast<entt::entity>(entity));
    }

    void Registry::Clear() {
        m_Registry.clear();
    }

} // namespace vyra::ecs
