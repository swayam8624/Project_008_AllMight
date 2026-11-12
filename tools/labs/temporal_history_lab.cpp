// Temporal-history reference lab: explicit history metadata, invalidation,
// reprojection confidence checks, disocclusion rejection and deterministic
// blending. This proves CPU logic, not a production TAA implementation.

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>

namespace allmight {

struct Resolution {
    int width{};
    int height{};
    bool operator==(const Resolution&) const = default;
};

struct HistoryMeta {
    Resolution resolution{};
    std::uint64_t camera_version{};
    std::uint64_t scene_version{};
    std::uint64_t projection_version{};
    int jitter_index{};
    bool valid{};
};

enum class RejectReason {
    none,
    no_history,
    resolution_change,
    camera_cut,
    scene_discontinuity,
    projection_change,
    invalid_motion,
    disocclusion,
    normal_mismatch
};

struct SurfaceSample {
    float depth{};
    float nx{}, ny{}, nz{};
};

float dot_normal(const SurfaceSample& a,const SurfaceSample& b){
    return a.nx*b.nx+a.ny*b.ny+a.nz*b.nz;
}

RejectReason validate_history(
    const HistoryMeta& previous,
    const HistoryMeta& current,
    const SurfaceSample& current_surface,
    const SurfaceSample& reprojected_previous_surface,
    bool motion_is_finite,
    float relative_depth_threshold = 0.02F,
    float normal_threshold = 0.8F) {

    if(!previous.valid) return RejectReason::no_history;
    if(!(previous.resolution==current.resolution)) return RejectReason::resolution_change;
    if(previous.camera_version!=current.camera_version) return RejectReason::camera_cut;
    if(previous.scene_version!=current.scene_version) return RejectReason::scene_discontinuity;
    if(previous.projection_version!=current.projection_version) return RejectReason::projection_change;
    if(!motion_is_finite) return RejectReason::invalid_motion;

    const float scale=std::max(1.0F,std::abs(current_surface.depth));
    if(std::abs(current_surface.depth-reprojected_previous_surface.depth) >
       relative_depth_threshold*scale) {
        return RejectReason::disocclusion;
    }

    if(dot_normal(current_surface,reprojected_previous_surface)<normal_threshold) {
        return RejectReason::normal_mismatch;
    }

    return RejectReason::none;
}

float clamp_history(float history,float neighborhood_min,float neighborhood_max){
    if(neighborhood_min>neighborhood_max) throw std::invalid_argument("invalid neighborhood interval");
    return std::max(neighborhood_min,std::min(history,neighborhood_max));
}

float blend(float current,float history,float history_weight){
    if(!(history_weight>=0.0F && history_weight<=1.0F)) throw std::invalid_argument("invalid history weight");
    return current*(1.0F-history_weight)+history*history_weight;
}

void tests(){
    const HistoryMeta prev{{1920,1080},7,12,3,4,true};
    const HistoryMeta curr{{1920,1080},7,12,3,5,true};
    const SurfaceSample a{10.0F,0,0,1};
    const SurfaceSample same{10.01F,0,0,1};

    assert(validate_history(prev,curr,a,same,true)==RejectReason::none);

    HistoryMeta changed=curr;
    changed.resolution={1280,720};
    assert(validate_history(prev,changed,a,same,true)==RejectReason::resolution_change);

    changed=curr;
    changed.camera_version=8;
    assert(validate_history(prev,changed,a,same,true)==RejectReason::camera_cut);

    changed=curr;
    changed.scene_version=13;
    assert(validate_history(prev,changed,a,same,true)==RejectReason::scene_discontinuity);

    const SurfaceSample behind{12.0F,0,0,1};
    assert(validate_history(prev,curr,a,behind,true)==RejectReason::disocclusion);

    const SurfaceSample flipped{10.0F,0,0,-1};
    assert(validate_history(prev,curr,a,flipped,true)==RejectReason::normal_mismatch);

    assert(validate_history(prev,curr,a,same,false)==RejectReason::invalid_motion);

    assert(std::abs(clamp_history(3.0F,0.5F,1.5F)-1.5F)<1.0e-6F);
    assert(std::abs(blend(1.0F,3.0F,0.25F)-1.5F)<1.0e-6F);
}

} // namespace allmight

int main(){
    allmight::tests();
    std::cout<<"temporal_history_lab: PASS\n";
    return 0;
}
