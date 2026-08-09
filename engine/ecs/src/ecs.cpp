#include "vyra/ecs/ecs.hpp"

namespace vyra::ecs {

    EntityID Registry::Create() {
        return m_Registry.create();
    }

    void Registry::Destroy(EntityID entity) {
        m_Registry.destroy(entity);
    }

    void Registry::Clear() {
        m_Registry.clear();
    }

} // namespace vyra::ecs
