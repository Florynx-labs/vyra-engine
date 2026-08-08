#pragma once

#include "vyra/core/base.hpp"
#include "vyra/scene/scene.hpp"
#include <glm/glm.hpp>
#include <imgui.h>

namespace vyra::editor {

    class VYRA_API ViewportPanel {
    public:
        ViewportPanel() = default;
        ~ViewportPanel() = default;

        void SetContext(Ref<vyra::scene::Scene> context);

        void OnImGuiRender(ImTextureID textureID = ImTextureID{});

        const glm::vec2& GetViewportSize() const { return m_ViewportSize; }
        bool IsFocused() const { return m_ViewportFocused; }
        bool IsHovered() const { return m_ViewportHovered; }

    private:
        Ref<vyra::scene::Scene> m_Context;

        glm::vec2 m_ViewportSize{ 1280.0f, 720.0f };
        glm::vec2 m_ViewportBounds[2];

        bool m_ViewportFocused{ false };
        bool m_ViewportHovered{ false };
    };

} // namespace vyra::editor
