#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct VyraEditorEngine VyraEditorEngine;
enum { VYRA_EDITOR_ENTITY_NAME_CAPACITY = 128 };
typedef struct VyraEditorEntityInfo { uint64_t id; char name[VYRA_EDITOR_ENTITY_NAME_CAPACITY]; } VyraEditorEntityInfo;
uint32_t vyra_editor_bridge_abi_version(void);
VyraEditorEngine* vyra_editor_engine_create(void);
void vyra_editor_engine_destroy(VyraEditorEngine* engine);
const char* vyra_editor_engine_last_error(const VyraEditorEngine* engine);
size_t vyra_editor_engine_entity_count(VyraEditorEngine* engine);
int32_t vyra_editor_engine_entity_at(VyraEditorEngine* engine, size_t index, VyraEditorEntityInfo* out_entity);
int32_t vyra_editor_engine_create_entity(VyraEditorEngine* engine, const char* name, uint64_t* out_entity_id);
int32_t vyra_editor_engine_rename_entity(VyraEditorEngine* engine, uint64_t entity_id, const char* name);
int32_t vyra_editor_engine_destroy_entity(VyraEditorEngine* engine, uint64_t entity_id);
int32_t vyra_editor_engine_clear_scene(VyraEditorEngine* engine);
#ifdef __cplusplus
}
#endif
