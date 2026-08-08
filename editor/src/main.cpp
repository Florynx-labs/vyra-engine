#include "vyra/core/base.hpp"
#include "vyra/core/log.hpp"
#include "vyra/platform/window.hpp"
#include "vyra/rhi/rhi.hpp"
#include "vyra/editor/editor_layer.hpp"

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <SDL3/SDL.h>

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    vyra::Log::Init();
    VYRA_LOG_INFO("Initializing VYRA Engine Editor v0.1...");

    // Create Platform Window
    vyra::WindowProps props("VYRA Engine Editor — Obsidian v0.1", 1600, 900);
    vyra::Scope<vyra::Window> window = vyra::Window::Create(props);

    bool running = true;
    window->SetEventCallback([&running](vyra::Event& e) {
        if (e.GetEventType() == vyra::EventType::WindowClose) {
            running = false;
        }
    });

    // Initialize ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Build ImGui Font Atlas
    unsigned char* fontPixels = nullptr;
    int fontWidth = 0, fontHeight = 0;
    io.Fonts->GetTexDataAsRGBA32(&fontPixels, &fontWidth, &fontHeight);
    io.Fonts->Build();

    // Attach Editor Layer
    vyra::editor::EditorLayer editorLayer;
    editorLayer.OnAttach();

    VYRA_LOG_INFO("VYRA Engine Editor running successfully.");

    vyra::Timestep ts(0.016f); // ~60 FPS frame time simulation

    while (running) {
        window->OnUpdate();

        // Update ImGui Display metrics
        io.DisplaySize = ImVec2(static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()));
        io.DeltaTime = ts.GetSeconds();

        editorLayer.OnUpdate(ts);

        // ImGui Frame rendering
        ImGui::NewFrame();
        editorLayer.OnImGuiRender();
        ImGui::Render();
    }

    editorLayer.OnDetach();
    ImGui::DestroyContext();
    vyra::Log::Shutdown();

    return 0;
}
