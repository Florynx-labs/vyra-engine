#include "vyra/editor/editor_layer.hpp"
#include "vyra/editor/editor_theme.hpp"
#include "vyra/scene/scene_serializer.hpp"
#include "vyra/scene/components.hpp"
#include "vyra/core/log.hpp"

#include <imgui.h>

namespace vyra::editor {

    EditorLayer::EditorLayer() {}

    void EditorLayer::OnAttach() {
        EditorTheme::ApplyObsidianDarkTheme();

        m_EditorScene = CreateRef<vyra::scene::Scene>("Sample World");
        m_ActiveScene = m_EditorScene;

        // Populate sample scene entities
        auto cameraEntity = m_EditorScene->CreateEntity("Main Camera");
        auto& cc = cameraEntity.AddComponent<vyra::scene::CameraComponent>();
        cc.Primary = true;

        auto cubeEntity = m_EditorScene->CreateEntity("Red Cube");
        auto& tc = cubeEntity.GetComponent<vyra::scene::TransformComponent>();
        tc.Translation = glm::vec3(0.0f, 0.0f, -5.0f);
        cubeEntity.AddComponent<vyra::scene::MeshComponent>("assets/models/cube.fbx", glm::vec4(0.9f, 0.2f, 0.2f, 1.0f));

        auto lightEntity = m_EditorScene->CreateEntity("Sun Light");
        lightEntity.AddComponent<vyra::scene::DirectionalLightComponent>();

        m_ViewportPanel.SetContext(m_ActiveScene);
        m_HierarchyPanel.SetContext(m_ActiveScene);

        m_EditorCamera = vyra::scene::EditorCamera(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

        m_ConsolePanel.AddMessage(LogMessage::Level::Info, "VYRA Engine Editor initialized successfully.");
        m_ConsolePanel.AddMessage(LogMessage::Level::Info, "Obsidian Dark UI Theme applied.");
    }

    void EditorLayer::OnDetach() {
    }

    void EditorLayer::OnUpdate(Timestep ts) {
        // Sync Viewport size to EditorCamera
        const auto& viewportSize = m_ViewportPanel.GetViewportSize();
        if (viewportSize.x > 0.0f && viewportSize.y > 0.0f) {
            m_EditorCamera.SetViewportSize(viewportSize.x, viewportSize.y);
        }

        if (m_ViewportPanel.IsHovered() || m_ViewportPanel.IsFocused()) {
            ImGuiIO& io = ImGui::GetIO();
            vyra::scene::CameraInput input;
            input.MouseDeltaX = io.MouseDelta.x;
            input.MouseDeltaY = io.MouseDelta.y;
            input.ScrollDelta = io.MouseWheel;
            input.RightMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);
            input.MiddleMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Middle);
            m_EditorCamera.OnUpdate(input, ts.GetSeconds());
        }

        if (m_SceneState == SceneState::Play) {
            m_ActiveScene->OnUpdateRuntime(ts);
        } else {
            m_ActiveScene->OnUpdateEditor(ts);
        }
    }

    void EditorLayer::OnImGuiRender() {
        // Configure DockSpace Window
        static bool dockspaceOpen = true;
        static bool opt_fullscreen = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen) {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("VYRA Engine Dockspace", &dockspaceOpen, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen) {
            ImGui::PopStyleVar(2);
        }

        // Submit DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("VyraDockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        UI_DrawMenuBar();
        UI_DrawToolbar();

        // Render Panels
        m_ViewportPanel.OnImGuiRender();
        m_HierarchyPanel.OnImGuiRender();
        m_InspectorPanel.OnImGuiRender(m_HierarchyPanel.GetSelectedEntity());
        m_ConsolePanel.OnImGuiRender();

        ImGui::End();
    }

    void EditorLayer::UI_DrawMenuBar() {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) NewScene();
                if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) OpenScene();
                if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) SaveSceneAs();
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {}
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Window")) {
                ImGui::MenuItem("Reset Layout", nullptr);
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    void EditorLayer::UI_DrawToolbar() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        auto& colors = ImGui::GetStyle().Colors;
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(colors[ImGuiCol_ButtonHovered].x, colors[ImGuiCol_ButtonHovered].y, colors[ImGuiCol_ButtonHovered].z, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(colors[ImGuiCol_ButtonActive].x, colors[ImGuiCol_ButtonActive].y, colors[ImGuiCol_ButtonActive].z, 0.5f));

        ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        const char* buttonText = m_SceneState == SceneState::Edit ? "▶ Play" : "⏹ Stop";
        if (ImGui::Button(buttonText, ImVec2(80, 24))) {
            if (m_SceneState == SceneState::Edit) {
                OnScenePlay();
            } else {
                OnSceneStop();
            }
        }

        ImGui::End();
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
    }

    void EditorLayer::NewScene() {
        m_EditorScene = CreateRef<vyra::scene::Scene>("Untitled Scene");
        m_ActiveScene = m_EditorScene;
        m_ViewportPanel.SetContext(m_ActiveScene);
        m_HierarchyPanel.SetContext(m_ActiveScene);
        m_ConsolePanel.AddMessage(LogMessage::Level::Info, "Created new empty scene.");
    }

    void EditorLayer::OpenScene() {
        std::string filepath = "scene.vyra";
        Ref<vyra::scene::Scene> newScene = CreateRef<vyra::scene::Scene>();
        vyra::scene::SceneSerializer serializer(newScene);
        if (serializer.Deserialize(filepath)) {
            m_EditorScene = newScene;
            m_ActiveScene = m_EditorScene;
            m_ViewportPanel.SetContext(m_ActiveScene);
            m_HierarchyPanel.SetContext(m_ActiveScene);
            m_ConsolePanel.AddMessage(LogMessage::Level::Info, "Loaded scene from '" + filepath + "'.");
        }
    }

    void EditorLayer::SaveSceneAs() {
        std::string filepath = "scene.vyra";
        vyra::scene::SceneSerializer serializer(m_ActiveScene);
        serializer.Serialize(filepath);
        m_ConsolePanel.AddMessage(LogMessage::Level::Info, "Saved scene to '" + filepath + "'.");
    }

    void EditorLayer::OnScenePlay() {
        m_SceneState = SceneState::Play;
        m_ActiveScene = vyra::scene::Scene::Copy(m_EditorScene);
        m_ViewportPanel.SetContext(m_ActiveScene);
        m_HierarchyPanel.SetContext(m_ActiveScene);
        m_ConsolePanel.AddMessage(LogMessage::Level::Info, "Entered Play Mode (World Isolated).");
    }

    void EditorLayer::OnSceneStop() {
        m_SceneState = SceneState::Edit;
        m_ActiveScene = m_EditorScene;
        m_ViewportPanel.SetContext(m_ActiveScene);
        m_HierarchyPanel.SetContext(m_ActiveScene);
        m_ConsolePanel.AddMessage(LogMessage::Level::Info, "Stopped Play Mode (Restored Editor World).");
    }

} // namespace vyra::editor
