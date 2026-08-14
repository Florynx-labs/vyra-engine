#include "vyra/scene/scene_serializer.hpp"
#include "vyra/scene/components.hpp"
#include "vyra/core/log.hpp"

#include <nlohmann/json.hpp>
#include <fstream>
#include <iomanip>
#include <filesystem>

namespace vyra::scene {

    using json = nlohmann::json;

    // Current scene format version
    static constexpr int CURRENT_FORMAT_VERSION = 2;

    // Migration functions for different format versions
    static bool MigrateFormatV1ToV2(json& root) {
        // Migration from version 1 to version 2
        // Example: Add new default fields or transform existing data
        
        if (!root.contains("Entities") || !root["Entities"].is_array()) {
            return true; // Nothing to migrate
        }
        
        for (auto& entityJson : root["Entities"]) {
            // Example migration: Add missing component versions
            if (entityJson.contains("TransformComponent")) {
                entityJson["TransformComponent"]["__version"] = 1;
            }
            if (entityJson.contains("CameraComponent")) {
                entityJson["CameraComponent"]["__version"] = 1;
            }
            // Add component versions for all components
            if (entityJson.contains("MeshComponent")) {
                entityJson["MeshComponent"]["__version"] = 1;
            }
            if (entityJson.contains("SpriteRendererComponent")) {
                entityJson["SpriteRendererComponent"]["__version"] = 1;
            }
            if (entityJson.contains("DirectionalLightComponent")) {
                entityJson["DirectionalLightComponent"]["__version"] = 1;
            }
            if (entityJson.contains("PointLightComponent")) {
                entityJson["PointLightComponent"]["__version"] = 1;
            }
        }
        
        // Update format version
        root["FormatVersion"] = 2;
        
        VYRA_LOG_CHANNEL(LogChannel::Serialization, info, "Migrated scene format from version 1 to version 2");
        return true;
    }
    
    static bool ApplyMigrations(json& root, int fromVersion, int toVersion) {
        if (fromVersion >= toVersion) {
            return true; // No migration needed
        }
        
        // Apply migrations sequentially
        for (int version = fromVersion; version < toVersion; version++) {
            switch (version) {
                case 1:
                    if (!MigrateFormatV1ToV2(root)) {
                        VYRA_LOG_CHANNEL(LogChannel::Serialization, error, "Failed to migrate from version {0} to {1}", version, version + 1);
                        return false;
                    }
                    break;
                default:
                    VYRA_LOG_CHANNEL(LogChannel::Serialization, warn, "No migration defined for version {0} to {1}", version, version + 1);
                    break;
            }
        }
        
        return true;
    }
    
    static bool ValidateComponentVersion(const json& componentJson, int expectedVersion) {
        if (!componentJson.contains("__version")) {
            // Older format without versioning, assume version 1
            return true;
        }
        
        int componentVersion = componentJson["__version"].get<int>();
        if (componentVersion > expectedVersion) {
            VYRA_LOG_CHANNEL(LogChannel::Serialization, warn, "Component version {0} is newer than expected {1}, attempting to load anyway", 
                             componentVersion, expectedVersion);
            // Try to load anyway - forward compatibility
            return true;
        }
        
        return componentVersion == expectedVersion;
    }

    SceneSerializer::SceneSerializer(Ref<Scene> scene)
        : m_Scene(scene) {}

    static json Vector3ToJSON(const glm::vec3& vec) {
        return json::array({ vec.x, vec.y, vec.z });
    }

    static bool TryReadVector3(const json& j, glm::vec3& out) {
        if (!j.is_array() || j.size() < 3) {
            return false;
        }

        out = glm::vec3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
        return true;
    }

    static glm::vec3 JSONToVector3(const json& j) {
        glm::vec3 result(0.0f);
        TryReadVector3(j, result);
        return result;
    }

    static bool TryReadVector4(const json& j, glm::vec4& out) {
        if (!j.is_array() || j.size() < 4) {
            return false;
        }

        out = glm::vec4(j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>());
        return true;
    }

    static glm::vec4 JSONToVector4(const json& j) {
        glm::vec4 result(1.0f);
        TryReadVector4(j, result);
        return result;
    }

    static json Vector4ToJSON(const glm::vec4& vec) {
        return json::array({ vec.x, vec.y, vec.z, vec.w });
    }

    static json SerializeEntity(vyra::ecs::Entity entity) {
        json entityJson;

        if (entity.HasComponent<TagComponent>()) {
            auto& tag = entity.GetComponent<TagComponent>();
            entityJson["Entity"] = tag.ID.ToString();
            entityJson["TagComponent"] = {
                { "Tag", tag.Tag }
            };
        }

        if (entity.HasComponent<TransformComponent>()) {
            auto& tc = entity.GetComponent<TransformComponent>();
            entityJson["TransformComponent"] = {
                { "Translation", Vector3ToJSON(tc.Translation) },
                { "Rotation", Vector3ToJSON(tc.Rotation) },
                { "Scale", Vector3ToJSON(tc.Scale) }
            };
        }

        if (entity.HasComponent<CameraComponent>()) {
            auto& cc = entity.GetComponent<CameraComponent>();
            auto& camera = cc.Camera;

            entityJson["CameraComponent"] = {
                { "Camera", {
                    { "ProjectionType", static_cast<int>(camera.GetProjectionType()) },
                    { "PerspectiveFOV", camera.GetPerspectiveVerticalFOV() },
                    { "PerspectiveNear", camera.GetPerspectiveNearClip() },
                    { "PerspectiveFar", camera.GetPerspectiveFarClip() },
                    { "OrthographicSize", camera.GetOrthographicSize() },
                    { "OrthographicNear", camera.GetOrthographicNearClip() },
                    { "OrthographicFar", camera.GetOrthographicFarClip() }
                }},
                { "Primary", cc.Primary },
                { "FixedAspectRatio", cc.FixedAspectRatio }
            };
        }

        if (entity.HasComponent<MeshComponent>()) {
            auto& mc = entity.GetComponent<MeshComponent>();
            entityJson["MeshComponent"] = {
                { "AssetPath", mc.AssetPath },
                { "Color", Vector4ToJSON(mc.Color) },
                { "MaterialID", mc.MaterialID }
            };
        }

        if (entity.HasComponent<SpriteRendererComponent>()) {
            auto& src = entity.GetComponent<SpriteRendererComponent>();
            entityJson["SpriteRendererComponent"] = {
                { "Color", Vector4ToJSON(src.Color) },
                { "TexturePath", src.TexturePath },
                { "TilingFactor", src.TilingFactor }
            };
        }

        if (entity.HasComponent<DirectionalLightComponent>()) {
            auto& dlc = entity.GetComponent<DirectionalLightComponent>();
            entityJson["DirectionalLightComponent"] = {
                { "Color", Vector3ToJSON(dlc.Color) },
                { "Intensity", dlc.Intensity }
            };
        }

        if (entity.HasComponent<PointLightComponent>()) {
            auto& plc = entity.GetComponent<PointLightComponent>();
            entityJson["PointLightComponent"] = {
                { "Color", Vector3ToJSON(plc.Color) },
                { "Intensity", plc.Intensity },
                { "Radius", plc.Radius }
            };
        }

        return entityJson;
    }

    void SceneSerializer::Serialize(const std::string& filepath) {
        json root;
        root["FormatVersion"] = CURRENT_FORMAT_VERSION;
        root["Scene"] = m_Scene->GetName();
        root["Entities"] = json::array();

        m_Scene->GetRegistry().Each<TagComponent>([&](vyra::ecs::EntityID entityID, TagComponent&) {
            vyra::ecs::Entity entity(entityID, &m_Scene->GetRegistry());
            if (entity) {
                root["Entities"].push_back(SerializeEntity(entity));
            }
        });

        std::ofstream out(filepath);
        if (out.is_open()) {
            out << root.dump(4);
            VYRA_LOG_CHANNEL(LogChannel::Serialization, info, "Serialized scene '{0}' to '{1}'", m_Scene->GetName(), filepath);
        } else {
            VYRA_LOG_CHANNEL(LogChannel::Serialization, error, "Failed to open file '{0}' for writing scene serialization", filepath);
        }
    }

    void SceneSerializer::SerializeRuntime(const std::string& filepath) {
        Serialize(filepath);
    }

    bool SceneSerializer::Deserialize(const std::string& filepath) {
        if (!std::filesystem::exists(filepath)) {
            VYRA_LOG_CHANNEL(LogChannel::Serialization, error, "Scene file does not exist: '{0}'", filepath);
            return false;
        }

        std::ifstream in(filepath);
        if (!in.is_open()) {
            VYRA_LOG_CHANNEL(LogChannel::Serialization, error, "Failed to open file '{0}' for scene deserialization", filepath);
            return false;
        }

        json root;
        try {
            in >> root;
        } catch (const std::exception& e) {
            VYRA_LOG_CHANNEL(LogChannel::Serialization, error, "JSON parse error deserializing scene '{0}': {1}", filepath, e.what());
            return false;
        }

        if (root.contains("FormatVersion")) {
            const int version = root["FormatVersion"].get<int>();
            if (version > CURRENT_FORMAT_VERSION) {
                VYRA_LOG_CHANNEL(LogChannel::Serialization, error, "Scene format version '{0}' is newer than supported version '{1}' while deserializing '{2}'", 
                                 version, CURRENT_FORMAT_VERSION, filepath);
                return false;
            }
            
            // Apply migrations if needed
            if (version < CURRENT_FORMAT_VERSION) {
                VYRA_LOG_CHANNEL(LogChannel::Serialization, info, "Migrating scene format from version {0} to {1}", version, CURRENT_FORMAT_VERSION);
                if (!ApplyMigrations(root, version, CURRENT_FORMAT_VERSION)) {
                    VYRA_LOG_CHANNEL(LogChannel::Serialization, error, "Failed to migrate scene format from version {0} to {1}", 
                                     version, CURRENT_FORMAT_VERSION);
                    return false;
                }
            }
        } else {
            // Old format without version, assume version 1
            VYRA_LOG_CHANNEL(LogChannel::Serialization, info, "Scene file has no format version, assuming version 1");
            root["FormatVersion"] = 1;
            if (!ApplyMigrations(root, 1, CURRENT_FORMAT_VERSION)) {
                VYRA_LOG_CHANNEL(LogChannel::Serialization, error, "Failed to migrate old format to current version");
                return false;
            }
        }

        m_Scene->Clear();

        if (root.contains("Scene")) {
            m_Scene->SetName(root["Scene"].get<std::string>());
        }

        if (!root.contains("Entities") || !root["Entities"].is_array()) {
            VYRA_LOG_CHANNEL(LogChannel::Serialization, warn, "Scene file has no entities: '{0}'", filepath);
            return true; // Not an error, just empty scene
        }

        for (const auto& entityJson : root["Entities"]) {
            std::string uuidStr = entityJson.value("Entity", std::string(""));
            UUID uuid = UUID::FromString(uuidStr);
            std::string name;
            if (entityJson.contains("TagComponent")) {
                name = entityJson["TagComponent"].value("Tag", "Entity");
            }

            vyra::ecs::EntityID entityID = m_Scene->GetRegistry().Create();
            vyra::ecs::Entity deserializedEntity(entityID, &m_Scene->GetRegistry());
            m_Scene->GetRegistry().Emplace<TagComponent>(entityID, name.empty() ? "Entity" : name, uuid);
            m_Scene->GetRegistry().Emplace<TransformComponent>(entityID);

            if (entityJson.contains("TransformComponent")) {
                const auto& transformJ = entityJson["TransformComponent"];
                
                // Validate component version
                if (!ValidateComponentVersion(transformJ, 1)) {
                    VYRA_LOG_CHANNEL(LogChannel::Serialization, warn, "TransformComponent version mismatch in entity '{0}'", name);
                }
                
                auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                if (transformJ.contains("Translation")) {
                    glm::vec3 translation(0.0f);
                    if (TryReadVector3(transformJ["Translation"], translation)) {
                        tc.Translation = translation;
                    }
                }
                if (transformJ.contains("Rotation")) {
                    glm::vec3 rotation(0.0f);
                    if (TryReadVector3(transformJ["Rotation"], rotation)) {
                        tc.Rotation = rotation;
                    }
                }
                if (transformJ.contains("Scale")) {
                    glm::vec3 scale(1.0f);
                    if (TryReadVector3(transformJ["Scale"], scale)) {
                        tc.Scale = scale;
                    }
                }
            }

            if (entityJson.contains("CameraComponent")) {
                auto& cc = deserializedEntity.AddComponent<CameraComponent>();
                const auto& cameraJ = entityJson["CameraComponent"];
                const auto& cameraDataJ = cameraJ["Camera"];
                
                if (cameraDataJ.contains("ProjectionType")) {
                    cc.Camera.SetProjectionType(static_cast<SceneCamera::ProjectionType>(cameraDataJ["ProjectionType"].get<int>()));
                }
                if (cameraDataJ.contains("PerspectiveFOV")) {
                    cc.Camera.SetPerspectiveVerticalFOV(cameraDataJ["PerspectiveFOV"].get<float>());
                }
                if (cameraDataJ.contains("PerspectiveNear")) {
                    cc.Camera.SetPerspectiveNearClip(cameraDataJ["PerspectiveNear"].get<float>());
                }
                if (cameraDataJ.contains("PerspectiveFar")) {
                    cc.Camera.SetPerspectiveFarClip(cameraDataJ["PerspectiveFar"].get<float>());
                }
                if (cameraDataJ.contains("OrthographicSize")) {
                    cc.Camera.SetOrthographicSize(cameraDataJ["OrthographicSize"].get<float>());
                }
                if (cameraDataJ.contains("OrthographicNear")) {
                    cc.Camera.SetOrthographicNearClip(cameraDataJ["OrthographicNear"].get<float>());
                }
                if (cameraDataJ.contains("OrthographicFar")) {
                    cc.Camera.SetOrthographicFarClip(cameraDataJ["OrthographicFar"].get<float>());
                }
                cc.Primary = cameraJ.value("Primary", true);
                cc.FixedAspectRatio = cameraJ.value("FixedAspectRatio", false);
            }

            if (entityJson.contains("MeshComponent")) {
                auto& mc = deserializedEntity.AddComponent<MeshComponent>();
                mc.AssetPath = entityJson["MeshComponent"].value("AssetPath", std::string(""));
                if (entityJson["MeshComponent"].contains("Color")) {
                    mc.Color = JSONToVector4(entityJson["MeshComponent"]["Color"]);
                }
                mc.MaterialID = entityJson["MeshComponent"].value("MaterialID", 0);
            }

            if (entityJson.contains("SpriteRendererComponent")) {
                auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
                if (entityJson["SpriteRendererComponent"].contains("Color")) {
                    src.Color = JSONToVector4(entityJson["SpriteRendererComponent"]["Color"]);
                }
                src.TexturePath = entityJson["SpriteRendererComponent"].value("TexturePath", std::string(""));
                src.TilingFactor = entityJson["SpriteRendererComponent"].value("TilingFactor", 1.0f);
            }

            if (entityJson.contains("DirectionalLightComponent")) {
                auto& dlc = deserializedEntity.AddComponent<DirectionalLightComponent>();
                if (entityJson["DirectionalLightComponent"].contains("Color")) {
                    dlc.Color = JSONToVector3(entityJson["DirectionalLightComponent"]["Color"]);
                }
                dlc.Intensity = entityJson["DirectionalLightComponent"].value("Intensity", 1.0f);
            }

            if (entityJson.contains("PointLightComponent")) {
                auto& plc = deserializedEntity.AddComponent<PointLightComponent>();
                if (entityJson["PointLightComponent"].contains("Color")) {
                    plc.Color = JSONToVector3(entityJson["PointLightComponent"]["Color"]);
                }
                plc.Intensity = entityJson["PointLightComponent"].value("Intensity", 1.0f);
                plc.Radius = entityJson["PointLightComponent"].value("Radius", 10.0f);
            }
        }

        VYRA_LOG_CHANNEL(LogChannel::Serialization, info, "Deserialized scene '{0}' from '{1}' with {2} entities", 
            m_Scene->GetName(), filepath, m_Scene->GetRegistry().Size());
        return true;
    }

} // namespace vyra::scene