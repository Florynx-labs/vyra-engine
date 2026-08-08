#pragma once

#include "vyra/core/base.hpp"
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace vyra::editor {

    struct LogMessage {
        enum class Level { Trace, Info, Warn, Error, Critical };

        std::string Text;
        Level Severity{ Level::Info };
        std::string Timestamp;
    };

    class VYRA_API ConsolePanel {
    public:
        ConsolePanel() = default;
        ~ConsolePanel() = default;

        void AddMessage(LogMessage::Level level, const std::string& text);
        void Clear();

        void OnImGuiRender();

    private:
        std::vector<LogMessage> m_Messages;
        bool m_AutoScroll{ true };
        bool m_ShowTrace{ true };
        bool m_ShowInfo{ true };
        bool m_ShowWarn{ true };
        bool m_ShowError{ true };
    };

} // namespace vyra::editor
