#pragma once

#include "vyra/core/base.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace vyra::animation {

struct Bone {
    std::string Name;
    int32_t Parent{-1};
    glm::mat4 InverseBind{1.0f};
};

struct Skeleton {
    std::vector<Bone> Bones;
    bool IsValid() const {
        for (std::size_t i = 0; i < Bones.size(); ++i) {
            const auto parent = Bones[i].Parent;
            if (parent < -1 || parent >= static_cast<int32_t>(Bones.size()) ||
                parent == static_cast<int32_t>(i)) return false;
        }
        return true;
    }
};

struct BonePose {
    glm::vec3 Translation{0.0f};
    glm::quat Rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 Scale{1.0f};
    glm::mat4 ToMatrix() const {
        return glm::translate(glm::mat4(1.0f), Translation) *
            glm::mat4_cast(glm::normalize(Rotation)) *
            glm::scale(glm::mat4(1.0f), Scale);
    }
};

struct AnimationClip {
    std::string Name;
    float DurationSeconds{0.0f};
    std::vector<std::vector<BonePose>> Frames;
    float FramesPerSecond{30.0f};

    bool IsValid(std::size_t boneCount) const {
        if (!std::isfinite(DurationSeconds) || DurationSeconds < 0.0f ||
            !std::isfinite(FramesPerSecond) || FramesPerSecond <= 0.0f) return false;
        for (const auto& frame : Frames) if (frame.size() != boneCount) return false;
        return true;
    }
};

// Samples between adjacent poses. This is the CPU-side foundation for a later
// animation graph and GPU skinning upload path.
inline std::vector<BonePose> Sample(const AnimationClip& clip, float timeSeconds, bool looping = true) {
    if (clip.Frames.empty()) return {};
    if (!std::isfinite(timeSeconds)) timeSeconds = 0.0f;
    const float framePosition = std::max(0.0f, timeSeconds) * clip.FramesPerSecond;
    const std::size_t last = clip.Frames.size() - 1;
    const float wrapped = looping && last > 0
        ? std::fmod(framePosition, static_cast<float>(clip.Frames.size()))
        : std::min(framePosition, static_cast<float>(last));
    const auto first = static_cast<std::size_t>(wrapped);
    const auto second = looping ? (first + 1) % clip.Frames.size() : std::min(first + 1, last);
    const float alpha = wrapped - static_cast<float>(first);
    std::vector<BonePose> result;
    result.reserve(clip.Frames[first].size());
    for (std::size_t i = 0; i < clip.Frames[first].size(); ++i) {
        const auto& a = clip.Frames[first][i];
        const auto& b = clip.Frames[second][i];
        BonePose pose;
        pose.Translation = glm::mix(a.Translation, b.Translation, alpha);
        pose.Rotation = glm::normalize(glm::slerp(a.Rotation, b.Rotation, alpha));
        pose.Scale = glm::mix(a.Scale, b.Scale, alpha);
        result.push_back(pose);
    }
    return result;
}

class VYRA_API BlendTree1D {
public:
    struct Child { float Threshold{0.0f}; std::vector<BonePose> Pose; };
    bool AddChild(float threshold, std::vector<BonePose> pose) {
        if (!std::isfinite(threshold) || pose.empty()) return false;
        auto it = std::lower_bound(m_Children.begin(), m_Children.end(), threshold,
            [](const Child& child, float value) { return child.Threshold < value; });
        if (it != m_Children.end() && it->Threshold == threshold) it->Pose = std::move(pose);
        else m_Children.insert(it, Child{threshold, std::move(pose)});
        return true;
    }
    std::vector<BonePose> Evaluate(float value) const {
        if (m_Children.empty()) return {};
        if (!std::isfinite(value)) value = 0.0f;
        auto upper = std::lower_bound(m_Children.begin(), m_Children.end(), value,
            [](const Child& child, float v) { return child.Threshold < v; });
        if (upper == m_Children.begin()) return upper->Pose;
        if (upper == m_Children.end()) return m_Children.back().Pose;
        const auto& b = *upper; const auto& a = *(upper - 1);
        if (a.Pose.size() != b.Pose.size()) return a.Pose;
        const float alpha = (value - a.Threshold) / (b.Threshold - a.Threshold);
        std::vector<BonePose> out; out.reserve(a.Pose.size());
        for (std::size_t i = 0; i < a.Pose.size(); ++i) {
            BonePose p;
            p.Translation = glm::mix(a.Pose[i].Translation, b.Pose[i].Translation, alpha);
            p.Rotation = glm::normalize(glm::slerp(a.Pose[i].Rotation, b.Pose[i].Rotation, alpha));
            p.Scale = glm::mix(a.Pose[i].Scale, b.Pose[i].Scale, alpha);
            out.push_back(p);
        }
        return out;
    }
private:
    std::vector<Child> m_Children;
};

} // namespace vyra::animation
