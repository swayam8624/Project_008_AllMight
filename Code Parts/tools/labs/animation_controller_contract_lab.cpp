// C++20 reference: clip evaluation, controller blending, root ownership and tick events.
// CPU contract only: no skeleton importer, inverse-bind matrices or GPU skinning.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace animation {
void check(bool ok, std::string_view problem) {
    if (!ok) throw std::runtime_error(std::string(problem));
}
struct Vec3 { double x = 0, y = 0, z = 0; };
Vec3 operator+(Vec3 a, Vec3 b) { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
Vec3 operator*(Vec3 v, double s) { return {v.x*s, v.y*s, v.z*s}; }
Vec3 interpolate(Vec3 a, Vec3 b, double t) { return a*(1-t)+b*t; }

// Quaternion convention: w + xi + yj + zk. A and -A represent the same
// spatial rotation, so shortest-arc slerp must change B's sign if dot < 0.
struct Quat { double w=1, x=0, y=0, z=0; };
double dot(Quat a, Quat b) {
    return a.w*b.w+a.x*b.x+a.y*b.y+a.z*b.z;
}
Quat operator-(Quat q) { return {-q.w,-q.x,-q.y,-q.z}; }
Quat operator+(Quat a, Quat b) {
    return {a.w+b.w,a.x+b.x,a.y+b.y,a.z+b.z};
}
Quat operator*(Quat q, double s) { return {q.w*s,q.x*s,q.y*s,q.z*s}; }
Quat normalized(Quat q) {
    const double squared=dot(q,q);
    check(std::isfinite(squared) && squared>1e-24, "invalid quaternion");
    return q*(1.0/std::sqrt(squared));
}
Quat slerp(Quat a, Quat b, double t) {
    check(std::isfinite(t) && t>=0 && t<=1, "invalid slerp weight");
    a=normalized(a); b=normalized(b);
    double cosine=dot(a,b);
    if (cosine<0) { b=-b; cosine=-cosine; }
    cosine=std::clamp(cosine,0.0,1.0);
    // Near equal rotations, linear blending avoids division by tiny sine.
    if (cosine>0.9995) return normalized(a*(1-t)+b*t);
    const double angle=std::acos(cosine);
    const double s=std::sin(angle);
    return normalized(a*(std::sin((1-t)*angle)/s)+
                      b*(std::sin(t*angle)/s));
}

struct JointPose { Vec3 translation; Quat rotation; };
JointPose blend(JointPose a, JointPose b, double w) {
    return {interpolate(a.translation,b.translation,w),slerp(a.rotation,b.rotation,w)};
}
struct Key { double time; JointPose pose; };
struct Marker { double time; std::string name; };
struct Clip {
    double duration = 1.0;             // Positive seconds per loop.
    double root_distance = 0.0;        // Meters per completed loop.
    std::vector<Key> foot_keys;        // One animated local joint in this model.
    std::vector<Marker> markers;       // Author-time semantic events.
};
void validate(const Clip& clip) {
    check(std::isfinite(clip.duration) && clip.duration>0, "bad clip duration");
    check(clip.foot_keys.size()>=2, "insufficient pose keys");
    check(std::isfinite(clip.root_distance), "bad root displacement");
    double previous=-1.0;
    for (const Key& k:clip.foot_keys) {
        check(std::isfinite(k.time) && k.time>=0 && k.time<=clip.duration &&
              k.time>previous, "pose keys must be sorted and unique");
        normalized(k.pose.rotation); // Reject invalid animation input early.
        previous=k.time;
    }
    check(clip.foot_keys.front().time==0 &&
          clip.foot_keys.back().time==clip.duration,
          "loop must have boundary pose keys");
    previous=-1.0;
    for (const Marker& marker:clip.markers) {
        check(marker.time>=0 && marker.time<clip.duration &&
              marker.time>previous && !marker.name.empty(), "invalid markers");
        previous=marker.time;
    }
}

JointPose pose_at(const Clip& clip, double absolute_seconds) {
    check(std::isfinite(absolute_seconds) && absolute_seconds>=0,
          "invalid clip time");
    const double t=std::fmod(absolute_seconds,clip.duration);
    // Inclusive right key means the pose remains defined for exact key times.
    auto upper=std::upper_bound(clip.foot_keys.begin(),clip.foot_keys.end(),t,
        [](double time, const Key& k) { return time<k.time; });
    if (upper==clip.foot_keys.begin()) return upper->pose;
    if (upper==clip.foot_keys.end()) return clip.foot_keys.back().pose;
    const Key& right=*upper; const Key& left=*(upper-1);
    const double alpha=(t-left.time)/(right.time-left.time);
    return blend(left.pose,right.pose,alpha);
}

// This simplified clip declares linear root movement over each loop. The
// unwrapped function makes a 0.99s -> 1.01s update advance, not jump backward.
double root_at(const Clip& clip,double absolute_seconds) {
    const double loops=std::floor(absolute_seconds/clip.duration);
    const double remainder=absolute_seconds-loops*clip.duration;
    return loops*clip.root_distance+
           (remainder/clip.duration)*clip.root_distance;
}
struct Event { std::string name; std::int64_t cycle; };
std::vector<Event> crossed_markers(const Clip& clip,double before,double after) {
    check(after>=before && std::isfinite(before) && std::isfinite(after),
          "backward or invalid event interval");
    check(after-before<=4*clip.duration, "event interval exceeds bounded update");
    std::vector<Event> fired;
    const auto first=static_cast<std::int64_t>(std::floor(before/clip.duration));
    const auto last=static_cast<std::int64_t>(std::floor(after/clip.duration));
    for (auto loop=first;loop<=last;++loop) {
        for (const Marker& marker:clip.markers) {
            const double when=loop*clip.duration+marker.time;
            if (when>before && when<=after)
                fired.push_back({marker.name,loop});
        }
    }
    return fired;
}

struct Advance {
    JointPose visual_foot;
    double requested_root_meters=0.0;
    std::vector<Event> events;
};
class Controller {
    const Clip& idle_;
    const Clip& walk_;
    bool walking_=false;
    double idle_time_=0,walk_time_=0;
    double transition_elapsed_=0;
    static constexpr double fade_seconds=0.20;
    std::uint64_t last_tick_=0;
public:
    Controller(const Clip& idle,const Clip& walk):idle_(idle),walk_(walk) {
        validate(idle_);validate(walk_);
    }
    void begin_walk() {
        check(!walking_, "duplicate transition request");
        walking_=true;transition_elapsed_=0;walk_time_=0;
    }
    Advance tick(std::uint64_t tick_id,double dt) {
        check(tick_id>last_tick_ && std::isfinite(dt) && dt>0,
              "tick must be positive and monotonic");
        check(dt<=0.10, "timestep exceeds animation contract");
        last_tick_=tick_id;
        const double old_walk=walk_time_;
        const double w0=walking_?std::clamp(transition_elapsed_/fade_seconds,0.0,1.0):0.0;
        idle_time_+=dt;
        if (!walking_) return {pose_at(idle_,idle_time_),0.0,{}};
        walk_time_+=dt;
        transition_elapsed_+=dt;
        const double w1=std::clamp(transition_elapsed_/fade_seconds,0.0,1.0);
        // Trapezoidal blend of movement weights is exact for a linear fade.
        // A nonlinear fade needs actual integration to avoid timestep bias.
        const double requested=(root_at(walk_,walk_time_)-root_at(walk_,old_walk))
                                *0.5*(w0+w1);
        auto pose=blend(pose_at(idle_,idle_time_),pose_at(walk_,walk_time_),w1);
        // Clip events are tick events; visual draws do not call tick().
        // A production controller must define event policy for multi-clip blends.
        return {pose,requested,crossed_markers(walk_,old_walk,walk_time_)};
    }
};

struct CharacterMotor {
    double authoritative_root_m=0;
    double previous_root_m=0;
    std::vector<std::string> dispatched_events;
    void commit(const Advance& proposal,double allowed_m) {
        check(std::isfinite(allowed_m) && allowed_m>=0,"bad collision allowance");
        previous_root_m=authoritative_root_m;
        authoritative_root_m+=std::min(proposal.requested_root_meters,allowed_m);
        for (const Event& e:proposal.events) dispatched_events.push_back(e.name);
    }
    double visual_root(double alpha) const {
        check(alpha>=0 && alpha<=1,"invalid presentation alpha");
        return previous_root_m*(1-alpha)+authoritative_root_m*alpha;
    }
};
} // namespace animation

int main() {
    using namespace animation;
    const Quat identity{};
    const Quat half_turn{0,0,0,1}; // 180 deg about local z.
    Clip idle{1,0,{{0,{{0,0,0},identity}},{1,{{0,0,0},identity}}}, {}};
    Clip walk{1,1,
        {{0,{{0,0,0},identity}},
         {0.5,{{0,0.2,0},half_turn}},
         {1,{{0,0,0},identity}}},
        {{0.25,"left-footstep"},{0.75,"right-footstep"}}};
    validate(walk);
    const JointPose at_half=pose_at(walk,0.5);
    check(std::abs(at_half.translation.y-0.2)<1e-12,"lost keyframe");
    const Quat quarter=slerp(identity,half_turn,0.5);
    check(std::abs(quarter.w-std::sqrt(0.5))<1e-12 &&
          std::abs(quarter.z-std::sqrt(0.5))<1e-12,
          "incorrect half-way quaternion orientation");
    check(std::abs(dot(quarter,slerp(identity,-half_turn,0.5)))-1<1e-12,
          "antipodal quaternion sign changed orientation");
    check(std::abs(root_at(walk,1.01)-root_at(walk,0.99)-0.02)<1e-12,
          "root movement snapped at loop boundary");
    const auto boundary_events=crossed_markers(walk,0.24,1.26);
    check(boundary_events.size()==3 && boundary_events[0].name=="left-footstep" &&
          boundary_events[2].cycle==1,"markers lost across loops");

    Controller controller(idle,walk);
    CharacterMotor motor;
    auto stand=controller.tick(1,0.1);
    check(stand.requested_root_meters==0 && stand.events.empty(),
          "idle state unexpectedly moved");
    controller.begin_walk();
    auto first=controller.tick(2,0.1);
    check(std::abs(first.requested_root_meters-0.025)<1e-12,
          "first half-fade root integral incorrect");
    motor.commit(first,1.0);
    const double before_draw=motor.authoritative_root_m;
    (void)motor.visual_root(0.5); // Presentation is read-only.
    (void)motor.visual_root(0.5);
    check(motor.authoritative_root_m==before_draw,"draw modified gameplay root");
    auto second=controller.tick(3,0.1);
    check(std::abs(second.requested_root_meters-0.075)<1e-12,
          "second half-fade root integral incorrect");
    motor.commit(second,0.05); // A wall permits less than the desired delta.
    check(std::abs(motor.authoritative_root_m-0.075)<1e-12,
          "root motion double-applied or ignored collision");
    auto third=controller.tick(4,0.1);
    motor.commit(third,1.0);
    check(motor.dispatched_events.size()==1 &&
          motor.dispatched_events[0]=="left-footstep",
          "footstep event must fire exactly once at the crossing tick");
    bool repeat_rejected=false;
    try { (void)controller.tick(4,0.1); }
    catch (const std::runtime_error&) {repeat_rejected=true;}
    check(repeat_rejected,"same animation tick executed twice");
    bool malformed_rejected=false;
    try { validate(Clip{1,1,{{0,{}},{0,{}}},{}}); }
    catch (const std::runtime_error&) {malformed_rejected=true;}
    check(malformed_rejected,"duplicate author-time keys accepted");
    std::cout << "PASS clip interpolation, shortest-arc blend, loop markers, "
                 "fade root integration, collision authority, tick events\n";
}
