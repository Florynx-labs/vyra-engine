#pragma once

#include "vyra/rhi/shader_abstraction.hpp"
#include <filesystem>
#include <fstream>

namespace vyra::rhi {

    // File-based shader cache implementation
    class FileShaderCache : public ShaderCache {
    public:
        explicit FileShaderCache(const std::string& cacheDirectory = "shader_cache");
        ~FileShaderCache() override = default;

        void Cache(const std::string& key, const ShaderArtifact& artifact) override;
        bool Load(const std::string& key, ShaderArtifact& outArtifact) override;
        bool IsCached(const std::string& key) const override;
        void Clear() override;
        size_t GetSize() const override;

    private:
        std::filesystem::path GetCachePath(const std::string& key) const;
        void EnsureCacheDirectory() const;

    private:
        std::filesystem::path m_CacheDirectory;
    };

} // namespace vyra::rhi