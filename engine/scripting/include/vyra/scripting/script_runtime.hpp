#pragma once

#include "vyra/core/base.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace vyra::scripting {

class ScriptContext {
public:
    virtual ~ScriptContext() = default;
};

class Script {
public:
    virtual ~Script() = default;
    virtual void OnCreate(ScriptContext&) {}
    virtual void OnUpdate(ScriptContext&, float) {}
    virtual void OnDestroy(ScriptContext&) {}
};

using ScriptFactory = std::function<std::unique_ptr<Script>()>;

class VYRA_API ScriptRegistry {
public:
    struct Entry { ScriptFactory Factory; uint64_t Generation{1}; };
    bool Register(std::string name, ScriptFactory factory) {
        if (name.empty() || !factory) return false;
        auto [it, inserted] = m_Entries.try_emplace(std::move(name), Entry{std::move(factory), 1});
        return inserted;
    }
    bool HotReload(const std::string& name, ScriptFactory replacement) {
        auto it = m_Entries.find(name);
        if (it == m_Entries.end() || !replacement) return false;
        it->second.Factory = std::move(replacement);
        ++it->second.Generation;
        return true;
    }
    std::unique_ptr<Script> Instantiate(const std::string& name) const {
        const auto it = m_Entries.find(name);
        return it == m_Entries.end() ? nullptr : it->second.Factory();
    }
    uint64_t GetGeneration(const std::string& name) const {
        const auto it = m_Entries.find(name);
        return it == m_Entries.end() ? 0 : it->second.Generation;
    }
    bool Contains(const std::string& name) const { return m_Entries.contains(name); }
private:
    std::unordered_map<std::string, Entry> m_Entries;
};

class VYRA_API ScriptInstance {
public:
    bool Attach(std::unique_ptr<Script> script, ScriptContext& context) {
        if (!script || m_Script) return false;
        m_Script = std::move(script);
        m_Script->OnCreate(context);
        m_Started = true;
        return true;
    }
    void Update(ScriptContext& context, float deltaSeconds) {
        if (m_Script && m_Started && deltaSeconds >= 0.0f) m_Script->OnUpdate(context, deltaSeconds);
    }
    void Detach(ScriptContext& context) {
        if (m_Script && m_Started) m_Script->OnDestroy(context);
        m_Script.reset();
        m_Started = false;
    }
    bool IsAttached() const { return static_cast<bool>(m_Script); }
private:
    std::unique_ptr<Script> m_Script;
    bool m_Started{false};
};

} // namespace vyra::scripting
