#pragma once

#include "vyra/core/base.hpp"
#include "vyra/core/time.hpp"
#include "vyra/scene/scene.hpp"
#include "vyra/editor/panels/viewport_panel.hpp"
#include "vyra/editor/panels/hierarchy_panel.hpp"
#include "vyra/editor/panels/inspector_panel.hpp"
#include "vyra/editor/panels/console_panel.hpp"

namespace vyra::editor {

    enum class SceneState { Edit = 0, Play = 1 };

    class VYRA_API EditorLayer {
    public:
        EditorLayer();
        ~EditorLayer() = default;

        void OnAttach();
        void OnDetach();

        void OnUpdate(Timestep ts);
        void OnImGuiRender();

    private:
        void NewScene();
        void OpenScene();
        void SaveSceneAs();

        void OnScenePlay();
        void OnSceneStop();

        void UI_DrawMenuBar();
        void UI_DrawToolbar();

    private:
        Ref<vyra::scene::Scene> m_EditorScene;
        Ref<vyra::scene::Scene> m_ActiveScene;

        SceneState m_SceneState{ SceneState::Edit };

        ViewportPanel m_ViewportPanel;
        HierarchyPanel m_HierarchyPanel;
        InspectorPanel m_InspectorPanel;
        ConsolePanel m_ConsolePanel;
    };

} // namespace vyra::editor
