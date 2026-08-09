#include "vyra/core/base.hpp"
#include "vyra/core/log.hpp"
#include "vyra/platform/window.hpp"
#include "vyra/rhi/rhi.hpp"
#include "vyra/editor/editor_layer.hpp"

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include <SDL3/SDL.h>

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    vyra::Log::Init();
    VYRA_LOG_INFO("Initializing VYRA Engine Editor v0.1...");

    // Create Platform Window
    vyra::WindowProps props("VYRA Engine Editor — Obsidian v0.1", 1600, 900);
    vyra::Scope<vyra::Window> window = vyra::Window::Create(props);
    SDL_Window* sdlWindow = static_cast<SDL_Window*>(window->GetNativeWindow());

    // Create SDL Hardware Renderer for Editor UI
    SDL_Renderer* sdlRenderer = SDL_CreateRenderer(sdlWindow, nullptr);
    if (!sdlRenderer) {
        VYRA_LOG_ERROR("Failed to create SDL_Renderer: {0}", SDL_GetError());
    }

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

    // Setup ImGui Backends
    ImGui_ImplSDL3_InitForSDLRenderer(sdlWindow, sdlRenderer);
    ImGui_ImplSDLRenderer3_Init(sdlRenderer);

    // Attach Editor Layer
    vyra::editor::EditorLayer editorLayer;
    editorLayer.OnAttach();

    VYRA_LOG_INFO("VYRA Engine Editor running successfully.");

    vyra::Timestep ts(0.016f); // ~60 FPS frame time simulation

    while (running) {
        window->OnUpdate();

        editorLayer.OnUpdate(ts);

        // ImGui Frame rendering
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        editorLayer.OnImGuiRender();

        ImGui::Render();

        if (sdlRenderer) {
            SDL_SetRenderDrawColor(sdlRenderer, 18, 18, 22, 255);
            SDL_RenderClear(sdlRenderer);
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), sdlRenderer);
            SDL_RenderPresent(sdlRenderer);
        }
    }

    editorLayer.OnDetach();
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    if (sdlRenderer) {
        SDL_DestroyRenderer(sdlRenderer);
    }
    vyra::Log::Shutdown();

    return 0;
}
