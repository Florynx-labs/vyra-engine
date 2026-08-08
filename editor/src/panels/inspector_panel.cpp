#include "vyra/editor/panels/inspector_panel.hpp"
#include "vyra/scene/components.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

namespace vyra::editor {

    static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f) {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        float lineHeight = ImGui::GetTextLineHeight() + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        // X Button & Drag
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        if (ImGui::Button("X", buttonSize)) values.x = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Y Button & Drag
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        if (ImGui::Button("Y", buttonSize)) values.y = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Z Button & Drag
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        if (ImGui::Button("Z", buttonSize)) values.z = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();
    }

    template<typename T, typename UIFunction>
    static void DrawComponentNode(const std::string& name, vyra::ecs::Entity entity, UIFunction uiFunction) {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_FramePadding;
        if (entity.HasComponent<T>()) {
            auto& component = entity.GetComponent<T>();
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
            float lineHeight = ImGui::GetTextLineHeight() + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();

            bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, "%s", name.c_str());
            ImGui::PopStyleVar();

            ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
            if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight })) {
                ImGui::OpenPopup("ComponentSettings");
            }

            bool removeComponent = false;
            if (ImGui::BeginPopup("ComponentSettings")) {
                if (ImGui::MenuItem("Remove Component")) {
                    removeComponent = true;
                }
                ImGui::EndPopup();
            }

            if (open) {
                uiFunction(component);
                ImGui::TreePop();
            }

            if (removeComponent) {
                entity.RemoveComponent<T>();
            }
        }
    }

    void InspectorPanel::OnImGuiRender(vyra::ecs::Entity selectedEntity) {
        ImGui::Begin("Inspector");

        if (selectedEntity) {
            DrawComponents(selectedEntity);
        } else {
            ImGui::TextColored(ImVec4(0.55f, 0.60f, 0.68f, 1.0f), "No entity selected.");
        }

        ImGui::End();
    }

    void InspectorPanel::DrawComponents(vyra::ecs::Entity entity) {
        if (entity.HasComponent<vyra::scene::TagComponent>()) {
            auto& tag = entity.GetComponent<vyra::scene::TagComponent>().Tag;

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy_s(buffer, tag.c_str(), sizeof(buffer));
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
                tag = std::string(buffer);
            }
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);

        if (ImGui::Button("Add Component")) {
            ImGui::OpenPopup("AddComponent");
        }

        if (ImGui::BeginPopup("AddComponent")) {
            if (!entity.HasComponent<vyra::scene::CameraComponent>() && ImGui::MenuItem("Camera")) {
                entity.AddComponent<vyra::scene::CameraComponent>();
                ImGui::CloseCurrentPopup();
            }
            if (!entity.HasComponent<vyra::scene::MeshComponent>() && ImGui::MenuItem("Mesh")) {
                entity.AddComponent<vyra::scene::MeshComponent>();
                ImGui::CloseCurrentPopup();
            }
            if (!entity.HasComponent<vyra::scene::SpriteRendererComponent>() && ImGui::MenuItem("Sprite Renderer")) {
                entity.AddComponent<vyra::scene::SpriteRendererComponent>();
                ImGui::CloseCurrentPopup();
            }
            if (!entity.HasComponent<vyra::scene::DirectionalLightComponent>() && ImGui::MenuItem("Directional Light")) {
                entity.AddComponent<vyra::scene::DirectionalLightComponent>();
                ImGui::CloseCurrentPopup();
            }
            if (!entity.HasComponent<vyra::scene::PointLightComponent>() && ImGui::MenuItem("Point Light")) {
                entity.AddComponent<vyra::scene::PointLightComponent>();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::PopItemWidth();

        DrawComponentNode<vyra::scene::TransformComponent>("Transform", entity, [](auto& component) {
            DrawVec3Control("Translation", component.Translation);
            glm::vec3 rotation = glm::degrees(component.Rotation);
            DrawVec3Control("Rotation", rotation);
            component.Rotation = glm::radians(rotation);
            DrawVec3Control("Scale", component.Scale, 1.0f);
        });

        DrawComponentNode<vyra::scene::CameraComponent>("Camera", entity, [](auto& component) {
            auto& camera = component.Camera;

            ImGui::Checkbox("Primary Camera", &component.Primary);
            ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);

            const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
            const char* currentProjectionTypeString = projectionTypeStrings[static_cast<int>(camera.GetProjectionType())];

            if (ImGui::BeginCombo("Projection", currentProjectionTypeString)) {
                for (int i = 0; i < 2; i++) {
                    bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
                    if (ImGui::Selectable(projectionTypeStrings[i], isSelected)) {
                        camera.SetProjectionType(static_cast<vyra::scene::SceneCamera::ProjectionType>(i));
                    }
                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            if (camera.GetProjectionType() == vyra::scene::SceneCamera::ProjectionType::Perspective) {
                float fov = glm::degrees(camera.GetPerspectiveVerticalFOV());
                if (ImGui::DragFloat("Vertical FOV", &fov)) camera.SetPerspectiveVerticalFOV(glm::radians(fov));

                float nearClip = camera.GetPerspectiveNearClip();
                if (ImGui::DragFloat("Near Clip", &nearClip)) camera.SetPerspectiveNearClip(nearClip);

                float farClip = camera.GetPerspectiveFarClip();
                if (ImGui::DragFloat("Far Clip", &farClip)) camera.SetPerspectiveFarClip(farClip);
            } else {
                float orthoSize = camera.GetOrthographicSize();
                if (ImGui::DragFloat("Size", &orthoSize)) camera.SetOrthographicSize(orthoSize);

                float nearClip = camera.GetOrthographicNearClip();
                if (ImGui::DragFloat("Near Clip", &nearClip)) camera.SetOrthographicNearClip(nearClip);

                float farClip = camera.GetOrthographicFarClip();
                if (ImGui::DragFloat("Far Clip", &farClip)) camera.SetOrthographicFarClip(farClip);
            }
        });

        DrawComponentNode<vyra::scene::MeshComponent>("Mesh Component", entity, [](auto& component) {
            char pathBuf[256];
            memset(pathBuf, 0, sizeof(pathBuf));
            strncpy_s(pathBuf, component.AssetPath.c_str(), sizeof(pathBuf));
            if (ImGui::InputText("Asset Path", pathBuf, sizeof(pathBuf))) {
                component.AssetPath = std::string(pathBuf);
            }
            ImGui::ColorEdit4("Mesh Color", glm::value_ptr(component.Color));
        });

        DrawComponentNode<vyra::scene::SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component) {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
            ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
        });

        DrawComponentNode<vyra::scene::DirectionalLightComponent>("Directional Light", entity, [](auto& component) {
            ImGui::ColorEdit3("Color", glm::value_ptr(component.Color));
            ImGui::DragFloat("Intensity", &component.Intensity, 0.1f, 0.0f, 100.0f);
        });

        DrawComponentNode<vyra::scene::PointLightComponent>("Point Light", entity, [](auto& component) {
            ImGui::ColorEdit3("Color", glm::value_ptr(component.Color));
            ImGui::DragFloat("Intensity", &component.Intensity, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Radius", &component.Radius, 0.1f, 0.1f, 1000.0f);
        });
    }

} // namespace vyra::editor
