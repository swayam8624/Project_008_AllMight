#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <vector>

struct Clip {
    std::vector<float> mono;
};

struct Voice {
    const Clip* clip = nullptr;
    std::size_t cursor = 0;
    float gain = 1.0f;
    int priority = 0;
    std::uint64_t age = 0;
    bool loop = false;
    bool active = false;
};

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

template <std::size_t N>
class Mixer {
public:
    // In a production audio callback, clip memory and voice storage must already
    // exist. This teaching mixer performs no allocation in mix_block().
    bool play(const Clip& clip, float gain, int priority, bool loop) {
        if (clip.mono.empty()) return false;

        Voice* slot = nullptr;
        for (auto& v : voices_) {
            if (!v.active) { slot = &v; break; }
        }

        if (!slot) {
            // Steal the least-important voice. For equal priority, steal the
            // oldest one so the rule is deterministic and inspectable.
            auto it = std::min_element(
                voices_.begin(), voices_.end(),
                [](const Voice& a, const Voice& b) {
                    if (a.priority != b.priority) return a.priority < b.priority;
                    return a.age < b.age;
                });
            if (priority < it->priority) return false;
            slot = &*it;
        }

        *slot = Voice{&clip, 0, gain, priority, next_age_++, loop, true};
        return true;
    }

    template <std::size_t Frames>
    std::array<float, Frames> mix_block() {
        std::array<float, Frames> out{};
        for (std::size_t frame = 0; frame < Frames; ++frame) {
            float sample = 0.0f;
            for (auto& voice : voices_) {
                if (!voice.active) continue;

                sample += voice.clip->mono[voice.cursor] * voice.gain;
                ++voice.cursor;

                if (voice.cursor == voice.clip->mono.size()) {
                    if (voice.loop) {
                        voice.cursor = 0;
                    } else {
                        voice.active = false;
                    }
                }
            }
            // This witness uses hard clipping only so the output contract is
            // bounded. A real engine may use headroom, limiters and mastering.
            out[frame] = std::clamp(sample, -1.0f, 1.0f);
        }
        return out;
    }

    std::size_t active_voices() const {
        return std::count_if(voices_.begin(), voices_.end(),
                             [](const Voice& v){ return v.active; });
    }

    bool has_priority(int p) const {
        return std::any_of(voices_.begin(), voices_.end(),
                           [=](const Voice& v){ return v.active && v.priority == p; });
    }

private:
    std::array<Voice, N> voices_{};
    std::uint64_t next_age_ = 0;
};

bool near(float a, float b, float eps=1e-6f) {
    return std::fabs(a-b) <= eps;
}

int main() {
    const Clip a{{0.25f, 0.25f, 0.25f}};
    const Clip b{{0.50f, -0.50f}};
    const Clip loop{{0.10f, 0.20f}};

    {
        Mixer<4> m;
        require(m.play(a, 1.0f, 1, false), "first voice should start");
        require(m.play(b, 0.5f, 2, false), "second voice should start");
        const auto out = m.mix_block<3>();
        require(near(out[0], 0.25f + 0.25f), "voices must sum into the same sample");
        require(near(out[1], 0.25f - 0.25f), "signed PCM samples must mix correctly");
        require(near(out[2], 0.25f), "finished short voice must stop contributing");
        require(m.active_voices() == 0, "non-looping voices should retire at end");
    }

    {
        Mixer<1> m;
        require(m.play(loop, 1.0f, 1, true), "looping voice should start");
        const auto out = m.mix_block<5>();
        require(near(out[0],0.10f) && near(out[1],0.20f) &&
                near(out[2],0.10f) && near(out[3],0.20f) && near(out[4],0.10f),
                "looping cursor must wrap exactly");
        require(m.active_voices() == 1, "loop remains active");
    }

    {
        Mixer<2> m;
        require(m.play(a, 1.0f, 1, true), "low-priority voice A");
        require(m.play(b, 1.0f, 5, true), "high-priority voice B");
        require(m.play(loop, 1.0f, 3, true), "new medium voice steals least priority");
        require(!m.has_priority(1) && m.has_priority(3) && m.has_priority(5),
                "voice stealing must preserve more important voices");
        require(!m.play(a, 1.0f, 0, true),
                "lower-priority request should be rejected when all slots are more important");
    }

    {
        Mixer<2> m;
        const Clip loud{{0.9f}};
        require(m.play(loud,1.0f,1,false) && m.play(loud,1.0f,1,false),
                "two loud voices should start");
        const auto out=m.mix_block<1>();
        require(near(out[0],1.0f), "output contract must remain bounded");
    }

    std::cout << "audio voice mixer: 4 suites PASS\n";
}
