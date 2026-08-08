#pragma once

#include "vyra/core/base.hpp"
#include "vyra/ecs/ecs.hpp"

namespace vyra::editor {

    class VYRA_API InspectorPanel {
    public:
        InspectorPanel() = default;
        ~InspectorPanel() = default;

        void OnImGuiRender(vyra::ecs::Entity selectedEntity);

    private:
        void DrawComponents(vyra::ecs::Entity entity);
    };

} // namespace vyra::editor
