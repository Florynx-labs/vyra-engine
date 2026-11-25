#pragma once

#include "vyra/core/base.hpp"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace vyra::audio {

struct AudioClip {
    uint32_t SampleRate{48000};
    uint16_t Channels{2};
    std::vector<float> Samples;
    bool IsValid() const {
        return SampleRate > 0 && (Channels == 1 || Channels == 2) &&
            !Samples.empty() && Samples.size() % Channels == 0;
    }
};

struct Voice {
    const AudioClip* Clip{nullptr};
    double CursorFrames{0.0};
    float Gain{1.0f};
    float Pan{0.0f};
    bool Looping{false};
    bool Playing{false};
};

struct Listener3D {
    glm::vec3 Position{0.0f};
    glm::vec3 Forward{0.0f, 0.0f, -1.0f};
    glm::vec3 Up{0.0f, 1.0f, 0.0f};
};

struct SpatialMix { float Gain{0.0f}; float Pan{0.0f}; };

struct Emitter3D {
    glm::vec3 Position{0.0f};
    float Gain{1.0f};
    float MinDistance{1.0f};
    float MaxDistance{100.0f};
};

class VYRA_API AudioMixer {
public:
    explicit AudioMixer(uint32_t outputRate = 48000) : m_OutputRate(outputRate ? outputRate : 48000) {}
    bool SetOutputRate(uint32_t rate) { if (!rate) return false; m_OutputRate = rate; return true; }
    uint32_t GetOutputRate() const { return m_OutputRate; }
    std::size_t AddVoice(Voice voice) { m_Voices.push_back(voice); return m_Voices.size() - 1; }
    bool StopVoice(std::size_t index) {
        if (index >= m_Voices.size()) return false;
        m_Voices[index].Playing = false; return true;
    }
    void ClearVoices() { m_Voices.clear(); }

    // Mixes interleaved stereo float samples. Backend device submission is separate.
    void Mix(float* output, std::size_t frameCount) {
        if (!output) return;
        std::fill(output, output + frameCount * 2, 0.0f);
        for (auto& voice : m_Voices) {
            if (!voice.Playing || !voice.Clip || !voice.Clip->IsValid() ||
                !std::isfinite(voice.Gain) || !std::isfinite(voice.Pan) ||
                !std::isfinite(voice.CursorFrames) || voice.CursorFrames < 0.0) continue;
            const auto& clip = *voice.Clip;
            const std::size_t totalFrames = clip.Samples.size() / clip.Channels;
            const float pan = std::clamp(voice.Pan, -1.0f, 1.0f);
            const float leftGain = voice.Gain * std::sqrt(0.5f * (1.0f - pan));
            const float rightGain = voice.Gain * std::sqrt(0.5f * (1.0f + pan));
            const double step = static_cast<double>(clip.SampleRate) / m_OutputRate;
            for (std::size_t frame = 0; frame < frameCount; ++frame) {
                std::size_t source = static_cast<std::size_t>(voice.CursorFrames);
                if (source >= totalFrames) {
                    if (voice.Looping) { voice.CursorFrames = std::fmod(voice.CursorFrames, static_cast<double>(totalFrames)); source = static_cast<std::size_t>(voice.CursorFrames); }
                    else { voice.Playing = false; break; }
                }
                const float left = clip.Samples[source * clip.Channels];
                const float right = clip.Channels == 2 ? clip.Samples[source * 2 + 1] : left;
                output[frame * 2] += left * leftGain;
                output[frame * 2 + 1] += right * rightGain;
                voice.CursorFrames += step;
            }
        }
        for (std::size_t i = 0; i < frameCount * 2; ++i)
            output[i] = std::clamp(output[i], -1.0f, 1.0f);
    }

    static SpatialMix Spatialize(const Listener3D& listener, const Emitter3D& emitter) {
        const glm::vec3 offset = emitter.Position - listener.Position;
        const float distance = glm::length(offset);
        if (!std::isfinite(distance) || distance <= 1.0e-6f) return {std::max(0.0f, emitter.Gain), 0.0f};
        const float forwardLength = glm::length(listener.Forward);
        const float upLength = glm::length(listener.Up);
        if (forwardLength <= 1.0e-6f || upLength <= 1.0e-6f || !std::isfinite(forwardLength) || !std::isfinite(upLength))
            return {Attenuation(listener, emitter), 0.0f};
        const glm::vec3 forward = listener.Forward / forwardLength;
        const glm::vec3 up = listener.Up / upLength;
        const glm::vec3 right = glm::cross(forward, up);
        const float rightLength = glm::length(right);
        const float pan = rightLength > 1.0e-6f ? glm::dot(offset / distance, right / rightLength) : 0.0f;
        return {Attenuation(listener, emitter), std::clamp(pan, -1.0f, 1.0f)};
    }

    static float Attenuation(const Listener3D& listener, const Emitter3D& emitter) {
        const float distance = glm::length(emitter.Position - listener.Position);
        if (!std::isfinite(distance) || emitter.MaxDistance <= emitter.MinDistance ||
            emitter.MinDistance < 0.0f) return 0.0f;
        const float t = std::clamp((distance - emitter.MinDistance) /
            (emitter.MaxDistance - emitter.MinDistance), 0.0f, 1.0f);
        return std::max(0.0f, emitter.Gain) * (1.0f - t);
    }

private:
    uint32_t m_OutputRate;
    std::vector<Voice> m_Voices;
};

} // namespace vyra::audio
