#include "vyra/editor/panels/viewport_panel.hpp"

namespace vyra::editor {

    void ViewportPanel::SetContext(Ref<vyra::scene::Scene> context) {
        m_Context = context;
    }

    void ViewportPanel::OnImGuiRender(ImTextureID textureID) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");

        auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
        auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
        auto viewportOffset = ImGui::GetWindowPos();
        m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
        m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        if (m_ViewportSize.x != viewportPanelSize.x || m_ViewportSize.y != viewportPanelSize.y) {
            m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
            if (m_Context && m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f) {
                m_Context->OnViewportResize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
            }
        }

        if (textureID != ImTextureID{}) {
            ImGui::Image(textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0, 1), ImVec2(1, 0));
        } else {
            // Display placeholder message when no texture is bound
            ImVec2 center = ImVec2(
                m_ViewportBounds[0].x + (m_ViewportSize.x * 0.5f) - 100.0f,
                m_ViewportBounds[0].y + (m_ViewportSize.y * 0.5f) - 10.0f
            );
            ImGui::SetCursorPos(ImVec2((m_ViewportSize.x * 0.5f) - 120.0f, (m_ViewportSize.y * 0.5f) - 15.0f));
            ImGui::TextColored(ImVec4(0.0f, 0.75f, 1.00f, 0.8f), "VYRA Viewport — Ready");
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

} // namespace vyra::editor
