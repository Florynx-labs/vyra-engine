#include "vyra/scene/scene_serializer.hpp"
#include "vyra/scene/components.hpp"
#include "vyra/core/log.hpp"

#include <nlohmann/json.hpp>
#include <fstream>
#include <iomanip>

namespace vyra::scene {

    using json = nlohmann::json;

    SceneSerializer::SceneSerializer(Ref<Scene> scene)
        : m_Scene(scene) {}

    static json Vector3ToJSON(const glm::vec3& vec) {
        return json::array({ vec.x, vec.y, vec.z });
    }

    static glm::vec3 JSONToVector3(const json& j) {
        return glm::vec3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
    }

    static json Vector4ToJSON(const glm::vec4& vec) {
        return json::array({ vec.x, vec.y, vec.z, vec.w });
    }

    static glm::vec4 JSONToVector4(const json& j) {
        return glm::vec4(j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>());
    }

    static json SerializeEntity(vyra::ecs::Entity entity) {
        json entityJson;

        if (entity.HasComponent<TagComponent>()) {
            auto& tag = entity.GetComponent<TagComponent>();
            entityJson["Entity"] = static_cast<uint64_t>(tag.ID);
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
            VYRA_LOG_INFO("Serialized scene '{0}' to '{1}'", m_Scene->GetName(), filepath);
        } else {
            VYRA_LOG_ERROR("Failed to open file '{0}' for writing scene serialization", filepath);
        }
    }

    void SceneSerializer::SerializeRuntime(const std::string& filepath) {
        Serialize(filepath);
    }

    bool SceneSerializer::Deserialize(const std::string& filepath) {
        std::ifstream in(filepath);
        if (!in.is_open()) {
            VYRA_LOG_ERROR("Failed to open file '{0}' for scene deserialization", filepath);
            return false;
        }

        json root;
        try {
            in >> root;
        } catch (const std::exception& e) {
            VYRA_LOG_ERROR("JSON parse error deserializing scene '{0}': {1}", filepath, e.what());
            return false;
        }

        if (root.contains("Scene")) {
            m_Scene->SetName(root["Scene"].get<std::string>());
        }

        if (!root.contains("Entities") || !root["Entities"].is_array()) {
            return true;
        }

        for (const auto& entityJson : root["Entities"]) {
            uint64_t uuid = entityJson.value("Entity", static_cast<uint64_t>(0));
            std::string name;
            if (entityJson.contains("TagComponent")) {
                name = entityJson["TagComponent"].value("Tag", "Entity");
            }

            vyra::ecs::Entity deserializedEntity = m_Scene->CreateEntityWithUUID(UUID(uuid), name);

            if (entityJson.contains("TransformComponent")) {
                auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                const auto& transformJ = entityJson["TransformComponent"];
                if (transformJ.contains("Translation")) tc.Translation = JSONToVector3(transformJ["Translation"]);
                if (transformJ.contains("Rotation")) tc.Rotation = JSONToVector3(transformJ["Rotation"]);
                if (transformJ.contains("Scale")) tc.Scale = JSONToVector3(transformJ["Scale"]);
            }

            if (entityJson.contains("CameraComponent")) {
                auto& cc = deserializedEntity.AddComponent<CameraComponent>();
                const auto& camJ = entityJson["CameraComponent"];
                cc.Primary = camJ.value("Primary", true);
                cc.FixedAspectRatio = camJ.value("FixedAspectRatio", false);

                if (camJ.contains("Camera")) {
                    const auto& cProps = camJ["Camera"];
                    int projType = cProps.value("ProjectionType", 0);
                    if (projType == 0) {
                        cc.Camera.SetPerspective(
                            cProps.value("PerspectiveFOV", glm::radians(45.0f)),
                            cProps.value("PerspectiveNear", 0.1f),
                            cProps.value("PerspectiveFar", 1000.0f)
                        );
                    } else {
                        cc.Camera.SetOrthographic(
                            cProps.value("OrthographicSize", 10.0f),
                            cProps.value("OrthographicNear", -1.0f),
                            cProps.value("OrthographicFar", 1.0f)
                        );
                    }
                }
            }

            if (entityJson.contains("MeshComponent")) {
                const auto& meshJ = entityJson["MeshComponent"];
                auto& mc = deserializedEntity.AddComponent<MeshComponent>();
                mc.AssetPath = meshJ.value("AssetPath", "");
                if (meshJ.contains("Color")) mc.Color = JSONToVector4(meshJ["Color"]);
                mc.MaterialID = meshJ.value("MaterialID", static_cast<uint32_t>(0));
            }

            if (entityJson.contains("SpriteRendererComponent")) {
                const auto& spriteJ = entityJson["SpriteRendererComponent"];
                auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
                if (spriteJ.contains("Color")) src.Color = JSONToVector4(spriteJ["Color"]);
                src.TexturePath = spriteJ.value("TexturePath", "");
                src.TilingFactor = spriteJ.value("TilingFactor", 1.0f);
            }

            if (entityJson.contains("DirectionalLightComponent")) {
                const auto& lightJ = entityJson["DirectionalLightComponent"];
                auto& dlc = deserializedEntity.AddComponent<DirectionalLightComponent>();
                if (lightJ.contains("Color")) dlc.Color = JSONToVector3(lightJ["Color"]);
                dlc.Intensity = lightJ.value("Intensity", 1.0f);
            }

            if (entityJson.contains("PointLightComponent")) {
                const auto& lightJ = entityJson["PointLightComponent"];
                auto& plc = deserializedEntity.AddComponent<PointLightComponent>();
                if (lightJ.contains("Color")) plc.Color = JSONToVector3(lightJ["Color"]);
                plc.Intensity = lightJ.value("Intensity", 1.0f);
                plc.Radius = lightJ.value("Radius", 10.0f);
            }
        }

        VYRA_LOG_INFO("Deserialized scene '{0}' from '{1}' ({2} entities)", m_Scene->GetName(), filepath, m_Scene->GetRegistry().Size());
        return true;
    }

    bool SceneSerializer::DeserializeRuntime(const std::string& filepath) {
        return Deserialize(filepath);
    }

} // namespace vyra::scene
