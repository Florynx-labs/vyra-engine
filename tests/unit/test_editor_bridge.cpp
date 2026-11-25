#include <catch2/catch_test_macros.hpp>
#include <vyra/editor_bridge/editor_bridge.h>
#include <cstring>
TEST_CASE("VYRA editor bridge exposes ABI version", "[editor_bridge]") { REQUIRE(vyra_editor_bridge_abi_version() == 1); }
TEST_CASE("VYRA editor bridge manages entities", "[editor_bridge]") {
 auto* engine = vyra_editor_engine_create(); REQUIRE(engine != nullptr); REQUIRE(vyra_editor_engine_entity_count(engine) == 0);
 uint64_t id = 0; REQUIRE(vyra_editor_engine_create_entity(engine, "Bridge Cube", &id) == 0); REQUIRE(vyra_editor_engine_entity_count(engine) == 1);
 VyraEditorEntityInfo info{}; REQUIRE(vyra_editor_engine_entity_at(engine, 0, &info) == 1); REQUIRE(info.id == id); REQUIRE(std::strcmp(info.name, "Bridge Cube") == 0);
 REQUIRE(vyra_editor_engine_rename_entity(engine, id, "Renamed Cube") == 0); REQUIRE(vyra_editor_engine_entity_at(engine, 0, &info) == 1); REQUIRE(std::strcmp(info.name, "Renamed Cube") == 0);
 REQUIRE(vyra_editor_engine_destroy_entity(engine, id) == 0); REQUIRE(vyra_editor_engine_entity_count(engine) == 0); vyra_editor_engine_destroy(engine);
}
TEST_CASE("VYRA editor bridge rejects invalid entity IDs", "[editor_bridge]") {
 auto* engine = vyra_editor_engine_create(); REQUIRE(engine != nullptr); REQUIRE(vyra_editor_engine_rename_entity(engine, 42, "Missing") == -1); REQUIRE(std::strlen(vyra_editor_engine_last_error(engine)) > 0); REQUIRE(vyra_editor_engine_destroy_entity(engine, 42) == -1); vyra_editor_engine_destroy(engine);
}
