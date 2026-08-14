#pragma once

#include "vyra/core/base.hpp"
#include <string>
#include <cstdint>
#include <regex>

namespace vyra {
namespace asset {

/**
 * @brief Asset type enumeration for identification
 */
enum class AssetType : uint8_t {
    Unknown = 0,
    Texture = 1,
    Mesh = 2,
    Material = 3,
    Shader = 4,
    Scene = 5,
    Audio = 6,
    Font = 7,
    Script = 8,
    Other = 255
};

/**
 * @brief Convert AssetType to string
 */
inline const char* AssetTypeToString(AssetType type) {
    switch (type) {
        case AssetType::Texture: return "texture";
        case AssetType::Mesh: return "mesh";
        case AssetType::Material: return "material";
        case AssetType::Shader: return "shader";
        case AssetType::Scene: return "scene";
        case AssetType::Audio: return "audio";
        case AssetType::Font: return "font";
        case AssetType::Script: return "script";
        case AssetType::Other: return "other";
        default: return "unknown";
    }
}

/**
 * @brief Convert string to AssetType
 */
inline AssetType StringToAssetType(const std::string& str) {
    if (str == "texture") return AssetType::Texture;
    if (str == "mesh") return AssetType::Mesh;
    if (str == "material") return AssetType::Material;
    if (str == "shader") return AssetType::Shader;
    if (str == "scene") return AssetType::Scene;
    if (str == "audio") return AssetType::Audio;
    if (str == "font") return AssetType::Font;
    if (str == "script") return AssetType::Script;
    if (str == "other") return AssetType::Other;
    return AssetType::Unknown;
}

/**
 * @brief Hierarchical asset identification system
 * 
 * AssetID provides collision-resistant asset identification through
 * path-based naming with type prefixes. Format: type:path/to/asset
 * Example: "textures:characters/player/default"
 */
class VYRA_API AssetID {
public:
    /**
     * @brief Default constructor - creates invalid asset ID
     */
    AssetID() : type_(AssetType::Unknown), path_("") {}
    
    /**
     * @brief Construct from type and path
     */
    AssetID(AssetType type, const std::string& path)
        : type_(type), path_(path) {
        NormalizePath();
    }
    
    /**
     * @brief Construct from string (format: "type:path")
     */
    explicit AssetID(const std::string& combined) {
        ParseFromString(combined);
    }
    
    /**
     * @brief Get asset type
     */
    AssetType GetType() const { return type_; }
    
    /**
     * @brief Get asset path
     */
    const std::string& GetPath() const { return path_; }
    
    /**
     * @brief Check if asset ID is valid
     */
    bool IsValid() const {
        return type_ != AssetType::Unknown && !path_.empty();
    }
    
    /**
     * @brief Convert to string (format: "type:path")
     */
    std::string ToString() const {
        if (!IsValid()) return "";
        return std::string(AssetTypeToString(type_)) + ":" + path_;
    }
    
    /**
     * @brief Get parent directory of asset path
     */
    std::string GetParentPath() const {
        if (path_.empty()) return "";
        
        size_t lastSlash = path_.find_last_of('/');
        if (lastSlash == std::string::npos) return "";
        
        return path_.substr(0, lastSlash);
    }
    
    /**
     * @brief Get asset name (last component of path)
     */
    std::string GetName() const {
        if (path_.empty()) return "";
        
        size_t lastSlash = path_.find_last_of('/');
        if (lastSlash == std::string::npos) return path_;
        
        return path_.substr(lastSlash + 1);
    }
    
    /**
     * @brief Create child asset ID (append to path)
     */
    AssetID CreateChild(const std::string& childName) const {
        std::string newPath = path_.empty() ? childName : path_ + "/" + childName;
        return AssetID(type_, newPath);
    }
    
    /**
     * @brief Comparison operators
     */
    bool operator==(const AssetID& other) const {
        return type_ == other.type_ && path_ == other.path_;
    }
    
    bool operator!=(const AssetID& other) const {
        return !(*this == other);
    }
    
    bool operator<(const AssetID& other) const {
        if (type_ != other.type_) return type_ < other.type_;
        return path_ < other.path_;
    }
    
    /**
     * @brief Hash function for AssetID
     */
    size_t Hash() const {
        return std::hash<std::string>()(ToString());
    }
    
    /**
     * @brief Create invalid asset ID
     */
    static AssetID Invalid() {
        return AssetID();
    }
    
private:
    /**
     * @brief Normalize path (remove redundant slashes, ensure forward slashes)
     */
    void NormalizePath() {
        // Convert backslashes to forward slashes
        for (char& c : path_) {
            if (c == '\\') c = '/';
        }
        
        // Remove redundant slashes
        std::string normalized;
        bool lastWasSlash = false;
        for (char c : path_) {
            if (c == '/') {
                if (!lastWasSlash) {
                    normalized += c;
                    lastWasSlash = true;
                }
            } else {
                normalized += c;
                lastWasSlash = false;
            }
        }
        
        // Remove trailing slash
        if (!normalized.empty() && normalized.back() == '/') {
            normalized.pop_back();
        }
        
        path_ = normalized;
    }
    
    /**
     * @brief Parse from string format "type:path"
     */
    void ParseFromString(const std::string& combined) {
        size_t colonPos = combined.find(':');
        if (colonPos == std::string::npos) {
            type_ = AssetType::Unknown;
            path_ = combined;
            return;
        }
        
        std::string typeStr = combined.substr(0, colonPos);
        path_ = combined.substr(colonPos + 1);
        type_ = StringToAssetType(typeStr);
        
        NormalizePath();
    }
    
    AssetType type_;
    std::string path_;
};

/**
 * @brief Hash function for AssetID (for use in unordered_map, etc.)
 */
struct AssetIDHash {
    std::size_t operator()(const AssetID& id) const {
        return id.Hash();
    }
};

} // namespace asset
} // namespace vyra
