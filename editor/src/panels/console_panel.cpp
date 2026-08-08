#include "vyra/editor/panels/console_panel.hpp"
#include <imgui.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace vyra::editor {

    static std::string GetCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");
        return ss.str();
    }

    void ConsolePanel::AddMessage(LogMessage::Level level, const std::string& text) {
        m_Messages.push_back({ text, level, GetCurrentTimestamp() });
    }

    void ConsolePanel::Clear() {
        m_Messages.clear();
    }

    void ConsolePanel::OnImGuiRender() {
        ImGui::Begin("Console");

        // Controls Header
        if (ImGui::Button("Clear")) Clear();
        ImGui::SameLine();
        ImGui::Checkbox("Auto-Scroll", &m_AutoScroll);
        ImGui::SameLine();

        ImGui::Checkbox("Trace", &m_ShowTrace);
        ImGui::SameLine();
        ImGui::Checkbox("Info", &m_ShowInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Warn", &m_ShowWarn);
        ImGui::SameLine();
        ImGui::Checkbox("Error", &m_ShowError);

        ImGui::Separator();

        ImGui::BeginChild("LogMessageRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        for (const auto& msg : m_Messages) {
            if (msg.Severity == LogMessage::Level::Trace && !m_ShowTrace) continue;
            if (msg.Severity == LogMessage::Level::Info && !m_ShowInfo) continue;
            if (msg.Severity == LogMessage::Level::Warn && !m_ShowWarn) continue;
            if ((msg.Severity == LogMessage::Level::Error || msg.Severity == LogMessage::Level::Critical) && !m_ShowError) continue;

            ImVec4 color = ImVec4(0.92f, 0.94f, 0.96f, 1.0f); // Default Info color
            switch (msg.Severity) {
                case LogMessage::Level::Trace:    color = ImVec4(0.55f, 0.60f, 0.68f, 1.0f); break;
                case LogMessage::Level::Info:     color = ImVec4(0.00f, 0.75f, 1.00f, 1.0f); break;
                case LogMessage::Level::Warn:     color = ImVec4(1.00f, 0.75f, 0.00f, 1.0f); break;
                case LogMessage::Level::Error:    color = ImVec4(1.00f, 0.25f, 0.25f, 1.0f); break;
                case LogMessage::Level::Critical: color = ImVec4(1.00f, 0.10f, 0.40f, 1.0f); break;
            }

            ImGui::TextColored(ImVec4(0.55f, 0.60f, 0.68f, 1.0f), "[%s]", msg.Timestamp.c_str());
            ImGui::SameLine();
            ImGui::TextColored(color, "%s", msg.Text.c_str());
        }

        if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();
        ImGui::End();
    }

} // namespace vyra::editor
