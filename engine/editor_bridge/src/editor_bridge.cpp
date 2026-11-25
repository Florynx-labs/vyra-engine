#include "vyra/editor_bridge/editor_bridge.h"
#include "vyra/scene/scene.hpp"

#include <algorithm>
#include <cstring>
#include <exception>
#include <string>

struct VyraEditorEngine {
    vyra::scene::Scene scene{"Untitled Scene"};
    std::string last_error;
};

namespace {
constexpr uint32_t kAbiVersion = 1;

void clear_error(VyraEditorEngine* engine) {
    if (engine) engine->last_error.clear();
}

int32_t fail(VyraEditorEngine* engine, const char* message) {
    if (engine) engine->last_error = message ? message : "Unknown VYRA editor bridge error";
    return -1;
}

void copy_string(char* destination, size_t capacity, const std::string& source) {
    if (!destination || capacity == 0) return;
    const size_t length = std::min(source.size(), capacity - 1);
    if (length > 0) std::memcpy(destination, source.data(), length);
    destination[length] = '\0';
}

entt::entity decode_entity_id(uint64_t value) {
    return static_cast<entt::entity>(static_cast<uint32_t>(value));
}
} // namespace

extern "C" {
uint32_t vyra_editor_bridge_abi_version(void) { return kAbiVersion; }

VyraEditorEngine* vyra_editor_engine_create(void) {
    try { return new VyraEditorEngine(); }
    catch (...) { return nullptr; }
}

void vyra_editor_engine_destroy(VyraEditorEngine* engine) { delete engine; }

const char* vyra_editor_engine_last_error(const VyraEditorEngine* engine) {
    return engine ? engine->last_error.c_str() : "Invalid VYRA editor engine handle";
}

size_t vyra_editor_engine_entity_count(VyraEditorEngine* engine) {
    if (!engine) return 0;
    clear_error(engine);
    try { return engine->scene.GetRegistry().Size(); }
    catch (const std::exception& error) { engine->last_error = error.what(); }
    catch (...) { engine->last_error = "Failed to count scene entities"; }
    return 0;
}

int32_t vyra_editor_engine_entity_at(VyraEditorEngine* engine, size_t index, VyraEditorEntityInfo* out_entity) {
    if (!engine || !out_entity) return fail(engine, "Invalid engine handle or entity output pointer");
    clear_error(engine);
    try {
        auto& registry = engine->scene.GetRegistry();
        size_t current_index = 0;
        for (const auto entity : engine->scene.GetAllEntitiesWith<vyra::scene::TagComponent>()) {
            if (current_index++ != index) continue;
            const auto& tag = registry.Get<vyra::scene::TagComponent>(entity);
            out_entity->id = static_cast<uint64_t>(static_cast<uint32_t>(entity));
            copy_string(out_entity->name, sizeof(out_entity->name), tag.Tag);
            return 1;
        }
        return 0;
    } catch (const std::exception& error) { return fail(engine, error.what()); }
    catch (...) { return fail(engine, "Failed to read scene entity"); }
}

int32_t vyra_editor_engine_create_entity(VyraEditorEngine* engine, const char* name, uint64_t* out_entity_id) {
    if (!engine || !out_entity_id) return fail(engine, "Invalid engine handle or entity ID output pointer");
    clear_error(engine);
    try {
        auto entity = engine->scene.CreateEntity(name ? name : "Entity");
        *out_entity_id = static_cast<uint64_t>(static_cast<uint32_t>(entity.GetID()));
        return 0;
    } catch (const std::exception& error) { return fail(engine, error.what()); }
    catch (...) { return fail(engine, "Failed to create scene entity"); }
}

int32_t vyra_editor_engine_rename_entity(VyraEditorEngine* engine, uint64_t entity_id, const char* name) {
    if (!engine || !name) return fail(engine, "Invalid engine handle or entity name");
    clear_error(engine);
    try {
        auto& registry = engine->scene.GetRegistry();
        const auto entity = decode_entity_id(entity_id);
        if (!registry.GetNativeRegistry().valid(entity)) return fail(engine, "Cannot rename an invalid scene entity");
        registry.Get<vyra::scene::TagComponent>(entity).Tag = name;
        return 0;
    } catch (const std::exception& error) { return fail(engine, error.what()); }
    catch (...) { return fail(engine, "Failed to rename scene entity"); }
}

int32_t vyra_editor_engine_destroy_entity(VyraEditorEngine* engine, uint64_t entity_id) {
    if (!engine) return fail(nullptr, "Invalid VYRA editor engine handle");
    clear_error(engine);
    try {
        auto& registry = engine->scene.GetRegistry();
        const auto native_id = decode_entity_id(entity_id);
        if (!registry.GetNativeRegistry().valid(native_id)) return fail(engine, "Cannot destroy an invalid scene entity");
        engine->scene.DestroyEntity(vyra::ecs::Entity(native_id, &registry));
        return 0;
    } catch (const std::exception& error) { return fail(engine, error.what()); }
    catch (...) { return fail(engine, "Failed to destroy scene entity"); }
}

int32_t vyra_editor_engine_clear_scene(VyraEditorEngine* engine) {
    if (!engine) return fail(nullptr, "Invalid VYRA editor engine handle");
    clear_error(engine);
    try { engine->scene.Clear(); return 0; }
    catch (const std::exception& error) { return fail(engine, error.what()); }
    catch (...) { return fail(engine, "Failed to clear scene"); }
}
} // extern "C"
