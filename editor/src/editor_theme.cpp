#include "vyra/editor/editor_theme.hpp"

namespace vyra::editor {

    void EditorTheme::ApplyObsidianDarkTheme() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Rounding & Spacing Settings
        style.WindowRounding = 6.0f;
        style.ChildRounding = 4.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;

        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.PopupBorderSize = 1.0f;

        style.ItemSpacing = ImVec2(8.0f, 6.0f);
        style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
        style.WindowPadding = ImVec2(10.0f, 10.0f);
        style.FramePadding = ImVec2(6.0f, 4.0f);

        // Obsidian Palette Definitions
        const ImVec4 bgDark         = ImVec4(0.06f, 0.07f, 0.09f, 1.00f); // #0f1217
        const ImVec4 bgPanel        = ImVec4(0.09f, 0.11f, 0.14f, 1.00f); // #171c24
        const ImVec4 bgHeader       = ImVec4(0.12f, 0.15f, 0.19f, 1.00f); // #1f2630
        const ImVec4 bgHover        = ImVec4(0.16f, 0.20f, 0.26f, 1.00f); // #293342
        const ImVec4 bgActive       = ImVec4(0.20f, 0.26f, 0.34f, 1.00f); // #334257

        const ImVec4 border         = ImVec4(0.16f, 0.20f, 0.26f, 0.60f);

        const ImVec4 textPrimary    = ImVec4(0.92f, 0.94f, 0.96f, 1.00f);
        const ImVec4 textSecondary  = ImVec4(0.55f, 0.60f, 0.68f, 1.00f);

        const ImVec4 accentPrimary   = ImVec4(0.00f, 0.75f, 1.00f, 1.00f); // Electric Cyan #00c0ff
        const ImVec4 accentHover     = ImVec4(0.20f, 0.82f, 1.00f, 1.00f);
        const ImVec4 accentActive    = ImVec4(0.00f, 0.65f, 0.90f, 1.00f);

        // Assign Colors
        colors[ImGuiCol_Text]                  = textPrimary;
        colors[ImGuiCol_TextDisabled]          = textSecondary;

        colors[ImGuiCol_WindowBg]              = bgDark;
        colors[ImGuiCol_ChildBg]               = bgPanel;
        colors[ImGuiCol_PopupBg]               = bgPanel;

        colors[ImGuiCol_Border]                = border;
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        colors[ImGuiCol_FrameBg]               = bgHeader;
        colors[ImGuiCol_FrameBgHovered]        = bgHover;
        colors[ImGuiCol_FrameBgActive]         = bgActive;

        colors[ImGuiCol_TitleBg]               = bgDark;
        colors[ImGuiCol_TitleBgActive]         = bgPanel;
        colors[ImGuiCol_TitleBgCollapsed]      = bgDark;

        colors[ImGuiCol_MenuBarBg]             = bgDark;

        colors[ImGuiCol_ScrollbarBg]           = bgDark;
        colors[ImGuiCol_ScrollbarGrab]         = bgHeader;
        colors[ImGuiCol_ScrollbarGrabHovered]  = bgHover;
        colors[ImGuiCol_ScrollbarGrabActive]   = accentPrimary;

        colors[ImGuiCol_CheckMark]             = accentPrimary;
        colors[ImGuiCol_SliderGrab]            = accentPrimary;
        colors[ImGuiCol_SliderGrabActive]      = accentActive;

        colors[ImGuiCol_Button]                = bgHeader;
        colors[ImGuiCol_ButtonHovered]         = bgHover;
        colors[ImGuiCol_ButtonActive]          = bgActive;

        colors[ImGuiCol_Header]                = bgHeader;
        colors[ImGuiCol_HeaderHovered]         = bgHover;
        colors[ImGuiCol_HeaderActive]          = bgActive;

        colors[ImGuiCol_Separator]             = border;
        colors[ImGuiCol_SeparatorHovered]      = accentPrimary;
        colors[ImGuiCol_SeparatorActive]       = accentActive;

        colors[ImGuiCol_ResizeGrip]            = bgHeader;
        colors[ImGuiCol_ResizeGripHovered]     = accentPrimary;
        colors[ImGuiCol_ResizeGripActive]      = accentActive;

        colors[ImGuiCol_Tab]                   = bgPanel;
        colors[ImGuiCol_TabHovered]            = bgHover;
        colors[ImGuiCol_TabActive]             = bgHeader;
        colors[ImGuiCol_TabUnfocused]          = bgDark;
        colors[ImGuiCol_TabUnfocusedActive]    = bgPanel;

        colors[ImGuiCol_DockingPreview]        = ImVec4(0.00f, 0.75f, 1.00f, 0.30f);
        colors[ImGuiCol_DockingEmptyBg]        = bgDark;

        colors[ImGuiCol_PlotLines]             = accentPrimary;
        colors[ImGuiCol_PlotLinesHovered]      = accentHover;
        colors[ImGuiCol_PlotHistogram]        = accentPrimary;
        colors[ImGuiCol_PlotHistogramHovered]   = accentHover;

        colors[ImGuiCol_TableHeaderBg]         = bgHeader;
        colors[ImGuiCol_TableBorderStrong]     = border;
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.16f, 0.20f, 0.26f, 0.30f);
        colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.02f);

        colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.00f, 0.75f, 1.00f, 0.35f);
        colors[ImGuiCol_DragDropTarget]        = accentPrimary;
        colors[ImGuiCol_NavHighlight]          = accentPrimary;
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
    }

} // namespace vyra::editor
