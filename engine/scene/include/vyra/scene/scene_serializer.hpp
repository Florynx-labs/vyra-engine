#pragma once

#include "vyra/core/base.hpp"
#include "vyra/scene/scene.hpp"

#include <string>

namespace vyra::scene {

    class VYRA_API SceneSerializer {
    public:
        SceneSerializer(Ref<Scene> scene);
        ~SceneSerializer() = default;

        void Serialize(const std::string& filepath);
        void SerializeRuntime(const std::string& filepath);

        bool Deserialize(const std::string& filepath);
        bool DeserializeRuntime(const std::string& filepath);

    private:
        Ref<Scene> m_Scene;
    };

} // namespace vyra::scene
