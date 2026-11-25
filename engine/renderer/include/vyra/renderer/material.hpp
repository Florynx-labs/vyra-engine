#pragma once

#include "vyra/core/base.hpp"
#include <glm/glm.hpp>
#include <cmath>

namespace vyra::renderer {

enum class TextureFormat : uint8_t { R8, RG8, RGBA8, RGBA8SRGB, RGBA16F, R32F, Depth32F };

struct VYRA_API TextureDescriptor {
    uint32_t Width{1};
    uint32_t Height{1};
    uint32_t MipLevels{1};
    uint32_t Layers{1};
    TextureFormat Format{TextureFormat::RGBA8};
    bool IsValid() const {
        return Width > 0 && Height > 0 && MipLevels > 0 && Layers > 0 &&
            Width <= 32768 && Height <= 32768 && MipLevels <= 16 && Layers <= 2048;
    }
};

struct TextureBinding {
    uint64_t AssetId{0};
    uint32_t UVSet{0};
    bool IsBound() const { return AssetId != 0 && UVSet <= 3; }
};

// CPU-side metallic/roughness material description. GPU descriptor allocation,
// texture streaming and shadow-map passes remain renderer-backend responsibilities.
struct VYRA_API PBRMaterial {
    glm::vec4 BaseColor{1.0f};
    float Metallic{0.0f};
    float Roughness{0.5f};
    float AmbientOcclusion{1.0f};
    glm::vec3 Emissive{0.0f};
    TextureBinding BaseColorMap;
    TextureBinding NormalMap;
    TextureBinding MetallicRoughnessMap;
    TextureBinding OcclusionMap;
    TextureBinding EmissiveMap;
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
            AlphaCutoff >= 0.0f && AlphaCutoff <= 1.0f &&
            (!BaseColorMap.AssetId || BaseColorMap.IsBound()) &&
            (!NormalMap.AssetId || NormalMap.IsBound()) &&
            (!MetallicRoughnessMap.AssetId || MetallicRoughnessMap.IsBound()) &&
            (!OcclusionMap.AssetId || OcclusionMap.IsBound()) &&
            (!EmissiveMap.AssetId || EmissiveMap.IsBound());
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
