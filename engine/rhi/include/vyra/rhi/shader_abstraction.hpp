#pragma once

#include "vyra/core/base.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace vyra::rhi {

    // Shader language types
    enum class ShaderLanguage {
        GLSL,           // Transitional - will be phased out
        Slang,          // Target shader language
        HLSL,           // For DirectX compatibility
        SPIRV           // Intermediate representation
    };

    // Shader stages
    enum class ShaderStage {
        Vertex = 0,
        Fragment = 1,
        Geometry = 2,
        TessControl = 3,
        TessEvaluation = 4,
        Compute = 5
    };

    // Shader source abstraction
    class VYRA_API ShaderSource {
    public:
        ShaderSource();
        ShaderSource(ShaderLanguage language, const std::string& source, ShaderStage stage);
        ~ShaderSource() = default;

        // Get shader source
        const std::string& GetSource() const { return m_Source; }
        void SetSource(const std::string& source) { m_Source = source; }

        // Get shader language
        ShaderLanguage GetLanguage() const { return m_Language; }
        void SetLanguage(ShaderLanguage language) { m_Language = language; }

        // Get shader stage
        ShaderStage GetStage() const { return m_Stage; }
        void SetStage(ShaderStage stage) { m_Stage = stage; }

        // Get entry point
        const std::string& GetEntryPoint() const { return m_EntryPoint; }
        void SetEntryPoint(const std::string& entryPoint) { m_EntryPoint = entryPoint; }

        // Add macro definition
        void AddMacro(const std::string& name, const std::string& value = "");
        const std::unordered_map<std::string, std::string>& GetMacros() const { return m_Macros; }

        // Add include path
        void AddIncludePath(const std::string& path);
        const std::vector<std::string>& GetIncludePaths() const { return m_IncludePaths; }

        // Load from file
        bool LoadFromFile(const std::string& filepath);

        // Validate shader source
        bool Validate() const;

    private:
        std::string m_Source;
        ShaderLanguage m_Language{ ShaderLanguage::GLSL };
        ShaderStage m_Stage{ ShaderStage::Vertex };
        std::string m_EntryPoint{ "main" };
        std::unordered_map<std::string, std::string> m_Macros;
        std::vector<std::string> m_IncludePaths;
    };

    // Shader compiler interface
    class VYRA_API ShaderCompiler {
    public:
        virtual ~ShaderCompiler() = default;

        // Compile shader source to SPIR-V
        virtual bool Compile(const ShaderSource& source, std::vector<uint32_t>& outSpirv) = 0;

        // Get compiler version
        virtual std::string GetVersion() const = 0;

        // Get supported languages
        virtual bool SupportsLanguage(ShaderLanguage language) const = 0;
    };

    // Shader artifact (compiled shader)
    class VYRA_API ShaderArtifact {
    public:
        ShaderArtifact();
        ShaderArtifact(std::vector<uint32_t> spirv, ShaderStage stage);
        ~ShaderArtifact() = default;

        // Get SPIR-V bytecode
        const std::vector<uint32_t>& GetSpirV() const { return m_SpirV; }
        void SetSpirV(const std::vector<uint32_t>& spirv) { m_SpirV = spirv; }

        // Get shader stage
        ShaderStage GetStage() const { return m_Stage; }
        void SetStage(ShaderStage stage) { m_Stage = stage; }

        // Get hash for caching
        size_t GetHash() const { return m_Hash; }
        void RecalculateHash();

        // Save to file
        bool SaveToFile(const std::string& filepath) const;

        // Load from file
        bool LoadFromFile(const std::string& filepath);

        // Validate artifact
        bool Validate() const;

    private:
        std::vector<uint32_t> m_Spirv;
        ShaderStage m_Stage{ ShaderStage::Vertex };
        size_t m_Hash{ 0 };
    };

    // Shader reflection data
    struct VYRA_API ShaderReflection {
        struct InputAttribute {
            uint32_t Location;
            uint32_t Format;
            uint32_t Size;
            std::string Name;
        };

        struct UniformBuffer {
            uint32_t Set;
            uint32_t Binding;
            uint32_t Size;
            std::string Name;
        };

        struct SampledImage {
            uint32_t Set;
            uint32_t Binding;
            std::string Name;
        };

        std::vector<InputAttribute> Inputs;
        std::vector<UniformBuffer> UniformBuffers;
        std::vector<SampledImage> SampledImages;
        uint32_t PushConstantsSize{ 0 };
    };

    // Shader reflection interface
    class VYRA_API ShaderReflector {
    public:
        virtual ~ShaderReflector() = default;

        // Reflect shader artifact
        virtual bool Reflect(const ShaderArtifact& artifact, ShaderReflection& outReflection) = 0;

        // Get supported stages
        virtual bool SupportsStage(ShaderStage stage) const = 0;
    };

    // Shader cache interface
    class VYRA_API ShaderCache {
    public:
        virtual ~ShaderCache() = default;

        // Cache shader artifact
        virtual void Cache(const std::string& key, const ShaderArtifact& artifact) = 0;

        // Load cached shader artifact
        virtual bool Load(const std::string& key, ShaderArtifact& outArtifact) = 0;

        // Check if shader is cached
        virtual bool IsCached(const std::string& key) const = 0;

        // Clear cache
        virtual void Clear() = 0;

        // Get cache size
        virtual size_t GetSize() const = 0;
    };

    // Shader manager - high-level shader management
    class VYRA_API ShaderManager {
    public:
        ShaderManager();
        ~ShaderManager();

        // Set compiler
        void SetCompiler(std::unique_ptr<ShaderCompiler> compiler);

        // Set reflector
        void SetReflector(std::unique_ptr<ShaderReflector> reflector);

        // Set cache
        void SetCache(std::unique_ptr<ShaderCache> cache);

        // Compile shader
        bool CompileShader(const ShaderSource& source, ShaderArtifact& outArtifact);

        // Reflect shader
        bool ReflectShader(const ShaderArtifact& artifact, ShaderReflection& outReflection);

        // Load and compile shader from file
        bool LoadShader(const std::string& filepath, ShaderStage stage, ShaderArtifact& outArtifact);

    private:
        std::unique_ptr<ShaderCompiler> m_Compiler;
        std::unique_ptr<ShaderReflector> m_Reflector;
        std::unique_ptr<ShaderCache> m_Cache;
    };

} // namespace vyra::rhi