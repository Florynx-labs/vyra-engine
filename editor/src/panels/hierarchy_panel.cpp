#include "vyra/editor/panels/hierarchy_panel.hpp"
#include "vyra/scene/components.hpp"
#include <imgui.h>

namespace vyra::editor {

    HierarchyPanel::HierarchyPanel(Ref<vyra::scene::Scene> context) {
        SetContext(context);
    }

    void HierarchyPanel::SetContext(Ref<vyra::scene::Scene> context) {
        m_Context = context;
        m_SelectionContext = {};
    }

    void HierarchyPanel::OnImGuiRender() {
        ImGui::Begin("Scene Hierarchy");

        if (m_Context) {
            m_Context->GetRegistry().Each<vyra::scene::TagComponent>([&](vyra::ecs::EntityID entityID, vyra::scene::TagComponent&) {
                vyra::ecs::Entity entity(entityID, &m_Context->GetRegistry());
                DrawEntityNode(entity);
            });

            // Right-click on blank space to create entity
            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
                m_SelectionContext = {};
            }

            if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
                if (ImGui::MenuItem("Create Empty Entity")) {
                    m_SelectionContext = m_Context->CreateEntity("Empty Entity");
                }
                if (ImGui::MenuItem("Create Camera")) {
                    auto camEntity = m_Context->CreateEntity("Camera");
                    camEntity.AddComponent<vyra::scene::CameraComponent>();
                    m_SelectionContext = camEntity;
                }
                if (ImGui::MenuItem("Create Cube Mesh")) {
                    auto cubeEntity = m_Context->CreateEntity("Cube Mesh");
                    cubeEntity.AddComponent<vyra::scene::MeshComponent>("assets/models/cube.fbx");
                    m_SelectionContext = cubeEntity;
                }
                if (ImGui::MenuItem("Create Directional Light")) {
                    auto lightEntity = m_Context->CreateEntity("Directional Light");
                    lightEntity.AddComponent<vyra::scene::DirectionalLightComponent>();
                    m_SelectionContext = lightEntity;
                }
                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }

    void HierarchyPanel::DrawEntityNode(vyra::ecs::Entity entity) {
        auto& tag = entity.GetComponent<vyra::scene::TagComponent>().Tag;

        ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.c_str());

        if (ImGui::IsItemClicked()) {
            m_SelectionContext = entity;
        }

        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete Entity")) {
                entityDeleted = true;
            }
            ImGui::EndPopup();
        }

        if (opened) {
            ImGui::TreePop();
        }

        if (entityDeleted) {
            m_Context->DestroyEntity(entity);
            if (m_SelectionContext == entity) {
                m_SelectionContext = {};
            }
        }
    }

} // namespace vyra::editor
