// PROJECTALLMIGHT Phase 26 reference: temporal validity, motion vectors, clustered depth slices, PCF and VSM.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

[[noreturn]] void fail(const std::string& m) { throw std::runtime_error(m); }
void require(bool c, const std::string& m) { if (!c) fail(m); }
bool near(float a, float b, float e = 1.0e-5F) { return std::fabs(a - b) <= e; }

struct V2 { float x = 0.0F, y = 0.0F; };
struct V3 { float x = 0.0F, y = 0.0F, z = 1.0F; };

float dot(V3 a, V3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

struct HistoryProbe {
    V2 previous_uv;
    float current_linear_depth = 0.0F;
    float previous_linear_depth = 0.0F;
    V3 current_normal;
    V3 previous_normal;
    std::uint32_t current_object = 0;
    std::uint32_t previous_object = 0;
    std::uint64_t current_history_generation = 0;
    std::uint64_t previous_history_generation = 0;
};

bool history_valid(
    const HistoryProbe& h,
    float relative_depth_tolerance,
    float normal_cosine_threshold) {

    require(relative_depth_tolerance >= 0.0F, "negative depth tolerance");
    require(normal_cosine_threshold >= -1.0F && normal_cosine_threshold <= 1.0F,
            "normal threshold outside cosine range");

    if (h.previous_uv.x < 0.0F || h.previous_uv.x > 1.0F ||
        h.previous_uv.y < 0.0F || h.previous_uv.y > 1.0F) {
        return false;
    }

    if (h.current_history_generation != h.previous_history_generation) return false;
    if (h.current_object != h.previous_object) return false;

    const float scale = std::max({1.0e-5F, std::fabs(h.current_linear_depth), std::fabs(h.previous_linear_depth)});
    if (std::fabs(h.current_linear_depth - h.previous_linear_depth) >
        relative_depth_tolerance * scale) {
        return false;
    }

    if (dot(h.current_normal, h.previous_normal) < normal_cosine_threshold) return false;
    return true;
}

V2 motion_vector(V2 current_uv, V2 previous_uv) {
    // Convention: sample previous history at current_uv + motion.
    return {previous_uv.x - current_uv.x, previous_uv.y - current_uv.y};
}

float temporal_blend(float current, float history, float history_weight, bool valid) {
    require(history_weight >= 0.0F && history_weight <= 1.0F, "history weight outside [0,1]");
    if (!valid) return current;
    return current * (1.0F - history_weight) + history * history_weight;
}

int logarithmic_depth_slice(float z, float near_z, float far_z, int slice_count) {
    require(near_z > 0.0F && far_z > near_z, "invalid cluster depth range");
    require(slice_count > 0, "slice count must be positive");
    if (z <= near_z) return 0;
    if (z >= far_z) return slice_count - 1;

    const float t = std::log(z / near_z) / std::log(far_z / near_z);
    const int slice = static_cast<int>(std::floor(t * static_cast<float>(slice_count)));
    return std::clamp(slice, 0, slice_count - 1);
}

float pcf_3x3(
    const std::array<float, 9>& shadow_depth,
    float receiver_depth,
    float bias) {

    require(bias >= 0.0F, "negative shadow bias");
    float visible = 0.0F;
    for (float stored : shadow_depth) {
        require(stored >= 0.0F && stored <= 1.0F, "shadow depth outside normalized range");
        visible += receiver_depth <= stored + bias ? 1.0F : 0.0F;
    }
    return visible / 9.0F;
}

float variance_shadow_upper_bound(
    float mean,
    float mean_square,
    float receiver_depth,
    float minimum_variance = 1.0e-6F) {

    require(minimum_variance > 0.0F, "minimum variance must be positive");
    if (receiver_depth <= mean) return 1.0F;

    const float variance = std::max(mean_square - mean * mean, minimum_variance);
    const float delta = receiver_depth - mean;
    return std::clamp(variance / (variance + delta * delta), 0.0F, 1.0F);
}

void test_temporal_validity() {
    HistoryProbe stable{
        {0.4F, 0.6F},
        5.0F, 5.02F,
        {0,0,1}, {0,0,1},
        17, 17,
        9, 9
    };
    require(history_valid(stable, 0.01F, 0.95F), "stable history should be accepted");

    HistoryProbe camera_cut = stable;
    camera_cut.current_history_generation = 10;
    require(!history_valid(camera_cut, 0.01F, 0.95F), "camera cut generation must invalidate history");

    HistoryProbe disocclusion = stable;
    disocclusion.previous_object = 88;
    require(!history_valid(disocclusion, 0.01F, 0.95F), "object mismatch must invalidate history");

    HistoryProbe depth_jump = stable;
    depth_jump.previous_linear_depth = 8.0F;
    require(!history_valid(depth_jump, 0.01F, 0.95F), "large depth mismatch must invalidate history");

    HistoryProbe offscreen = stable;
    offscreen.previous_uv = {-0.01F, 0.5F};
    require(!history_valid(offscreen, 0.01F, 0.95F), "out-of-bounds reprojection must invalidate history");

    require(near(temporal_blend(10.0F, 2.0F, 0.9F, false), 10.0F),
            "invalid history must not contaminate current sample");
    require(near(temporal_blend(10.0F, 2.0F, 0.75F, true), 4.0F),
            "valid temporal blend trace mismatch");
}

void test_motion_vector_convention() {
    const V2 m = motion_vector({0.6F, 0.5F}, {0.55F, 0.48F});
    require(near(0.6F + m.x, 0.55F) && near(0.5F + m.y, 0.48F),
            "motion vector must map current UV to previous UV under declared convention");
}

void test_cluster_slices() {
    require(logarithmic_depth_slice(0.01F, 0.1F, 1000.0F, 16) == 0, "near clamp failed");
    require(logarithmic_depth_slice(2000.0F, 0.1F, 1000.0F, 16) == 15, "far clamp failed");

    int previous = -1;
    for (int i = 0; i <= 1000; ++i) {
        const float t = static_cast<float>(i) / 1000.0F;
        const float z = 0.1F * std::pow(10000.0F, t);
        const int slice = logarithmic_depth_slice(z, 0.1F, 1000.0F, 16);
        require(slice >= previous, "logarithmic cluster slice must be monotone with depth");
        previous = slice;
    }

    // Multiplicative depth ratios occupy equal-width regions in log space.
    const int a = logarithmic_depth_slice(1.0F, 0.1F, 1000.0F, 16);
    const int b = logarithmic_depth_slice(10.0F, 0.1F, 1000.0F, 16);
    const int c = logarithmic_depth_slice(100.0F, 0.1F, 1000.0F, 16);
    require((b - a) == (c - b), "equal depth decades should span equal log-slice counts");
}

void test_shadow_filters() {
    const std::array<float,9> map{
        0.4F,0.4F,0.7F,
        0.4F,0.7F,0.7F,
        0.4F,0.7F,0.7F
    };
    const float p = pcf_3x3(map, 0.5F, 0.0F);
    require(near(p, 5.0F/9.0F), "PCF visibility fraction mismatch");

    const float mean = 0.4F;
    const float variance = 0.01F;
    const float mean_square = variance + mean*mean;
    const float near_receiver = variance_shadow_upper_bound(mean, mean_square, 0.5F);
    const float far_receiver = variance_shadow_upper_bound(mean, mean_square, 0.8F);
    require(near_receiver > far_receiver, "VSM Chebyshev bound should decrease as receiver moves beyond mean");
    require(near_receiver >= 0.0F && near_receiver <= 1.0F &&
            far_receiver >= 0.0F && far_receiver <= 1.0F,
            "VSM bound outside probability range");
}

} // namespace

int main() {
    try {
        test_temporal_validity();
        test_motion_vector_convention();
        test_cluster_slices();
        test_shadow_filters();
        std::cout
            << "realtime_rendering_reference_lab: PASS\n"
            << "proved: explicit temporal-history invalidation, motion-vector convention, "
               "logarithmic clustered depth slicing, 3x3 PCF fraction, and VSM/Chebyshev monotonic bound\n"
            << "not proved: full TAA neighborhood clipping, jitter sequence, screen-space reprojection, "
               "cluster light-volume intersection, cascaded shadows, SSR/SSAO, volumetrics, particles, "
               "GPU execution, or production renderer parity\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "realtime_rendering_reference_lab: FAIL: " << e.what() << '\n';
        return 1;
    }
}
