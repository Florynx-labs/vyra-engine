#pragma once

#include "vyra/core/base.hpp"
#include "vyra/scene/scene.hpp"
#include "vyra/ecs/ecs.hpp"

namespace vyra::editor {

    class VYRA_API HierarchyPanel {
    public:
        HierarchyPanel() = default;
        HierarchyPanel(Ref<vyra::scene::Scene> context);
        ~HierarchyPanel() = default;

        void SetContext(Ref<vyra::scene::Scene> context);

        void OnImGuiRender();

        vyra::ecs::Entity GetSelectedEntity() const { return m_SelectionContext; }
        void SetSelectedEntity(vyra::ecs::Entity entity) { m_SelectionContext = entity; }

    private:
        void DrawEntityNode(vyra::ecs::Entity entity);

    private:
        Ref<vyra::scene::Scene> m_Context;
        vyra::ecs::Entity m_SelectionContext;
    };

} // namespace vyra::editor
