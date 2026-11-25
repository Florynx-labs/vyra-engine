#pragma once

#include "vyra/core/base.hpp"
#include <glm/glm.hpp>
#include <cmath>

namespace vyra::renderer {

// CPU-side metallic/roughness material description. GPU descriptor allocation,
// texture streaming and shadow-map passes remain renderer-backend responsibilities.
struct VYRA_API PBRMaterial {
    glm::vec4 BaseColor{1.0f};
    float Metallic{0.0f};
    float Roughness{0.5f};
    float AmbientOcclusion{1.0f};
    glm::vec3 Emissive{0.0f};
    float AlphaCutoff{0.5f};
    bool AlphaMasked{false};
    bool DoubleSided{false};

    bool IsValid() const {
        const auto finite = [](float v) { return std::isfinite(v); };
        return finite(BaseColor.x) && finite(BaseColor.y) && finite(BaseColor.z) &&
            finite(BaseColor.w) && finite(Metallic) && finite(Roughness) &&
            finite(AmbientOcclusion) && finite(Emissive.x) && finite(Emissive.y) &&
            finite(Emissive.z) && finite(AlphaCutoff) &&
            Metallic >= 0.0f && Metallic <= 1.0f &&
            Roughness >= 0.0f && Roughness <= 1.0f &&
            AmbientOcclusion >= 0.0f && AmbientOcclusion <= 1.0f &&
            AlphaCutoff >= 0.0f && AlphaCutoff <= 1.0f;
    }
};

struct VYRA_API RenderFeatureSet {
    bool Textures{true};
    bool PBRMaterials{true};
    bool DirectionalShadows{true};
    bool PointShadows{false};
    uint32_t ShadowMapResolution{2048};
    uint32_t MaxShadowCascades{4};

    bool IsValid() const {
        return ShadowMapResolution >= 128 && ShadowMapResolution <= 16384 &&
            MaxShadowCascades >= 1 && MaxShadowCascades <= 8;
    }
};

} // namespace vyra::renderer
