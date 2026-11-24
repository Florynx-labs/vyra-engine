#include "vyra/ecs/ecs.hpp"

namespace vyra::ecs {

    EntityID Registry::Create() {
        return m_State->NativeRegistry.create();
    }

    void Registry::Destroy(EntityID entity) {
        if (m_State->NativeRegistry.valid(entity)) {
            m_State->NativeRegistry.destroy(entity);
        }
    }

    void Registry::Clear() {
        m_State->NativeRegistry.clear();
    }

} // namespace vyra::ecs
