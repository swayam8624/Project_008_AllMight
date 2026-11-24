#pragma once
// P11: a small, fully inspectable affine scene forest. No graphics API.
// Matrices are represented by their COLUMNS, and act on column vectors.
// Every local map takes node-local coordinates to its parent's coordinates.
// Asset IDs are stable indices into immutable shared geometry during evaluation.
// This is a bounded forest, NOT a multiple-parent DAG or animation/skin system.
#include "book03/mesh_topology.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace book03 {
struct Linear3 {
    Vector3 x{1,0,0},y{0,1,0},z{0,0,1};
    [[nodiscard]] Vector3 apply(Vector3 v) const {
        const auto q=v.x*x+v.y*y+v.z*z;
        if(!finite(q))throw std::overflow_error("nonfinite linear result");
        return q;
    }
    [[nodiscard]] double determinant() const {return dot(x,cross(y,z));}
    [[nodiscard]] Linear3 transpose() const {
        return {{x.x,y.x,z.x},{x.y,y.y,z.y},{x.z,y.z,z.z}};
    }
    [[nodiscard]] Linear3 inverse() const {
        // Scale-normalized inverse. A relative determinant threshold is a
        // deliberate numerical policy, not a certified condition-number test.
        const double s=std::max({magnitude(x),magnitude(y),magnitude(z)});
        if(!(s>0)||!std::isfinite(s))throw std::domain_error("singular linear map");
        const Vector3 a=(1/s)*x,b=(1/s)*y,c=(1/s)*z;
        const double d=dot(a,cross(b,c));
        if(!std::isfinite(d)||std::abs(d)<=1e-12)
            throw std::domain_error("singular or ill-conditioned linear map");
        const double k=(1/d)/s;
        const Linear3 rows{k*cross(b,c),k*cross(c,a),k*cross(a,b)};
        const auto inv=rows.transpose();
        if(!finite(inv.x)||!finite(inv.y)||!finite(inv.z))
            throw std::overflow_error("inverse not representable");
        return inv;
    }
};
[[nodiscard]] inline Linear3 operator*(const Linear3& a,const Linear3& b) {
    return {a.apply(b.x),a.apply(b.y),a.apply(b.z)};
}
struct SceneAffine {
    Linear3 linear{};
    Vector3 translation{};
    [[nodiscard]] Point3 point(Point3 p) const {
        const auto v=linear.apply({p.x,p.y,p.z})+translation;
        if(!finite(v))throw std::overflow_error("nonfinite scene point");
        return {v.x,v.y,v.z};
    }
    [[nodiscard]] Vector3 tangent(Vector3 v) const {return linear.apply(v);}
    [[nodiscard]] Vector3 normal(Vector3 n) const {
        // A transformed normal is a covector, not an ordinary tangent.
        // Reflections reverse triangle winding; callers choose culling policy.
        return unit(linear.inverse().transpose().apply(n));
    }
    [[nodiscard]] SceneAffine inverse() const {
        const auto inv=linear.inverse();
        return {inv,-inv.apply(translation)};
    }
};
[[nodiscard]] inline SceneAffine operator*(const SceneAffine& a,
                                             const SceneAffine& b) {
    // a after b: (A(Bp+b)+a) = ABp+(Ab+a).
    const auto t=a.linear.apply(b.translation)+a.translation;
    if(!finite(t))throw std::overflow_error("nonfinite composition");
    return {a.linear*b.linear,t};
}
[[nodiscard]] inline SceneAffine scene_translate(Vector3 t) {
    if(!finite(t))throw std::invalid_argument("nonfinite translation");
    return {{},t};
}
[[nodiscard]] inline SceneAffine scene_scale(Vector3 s) {
    if(!finite(s))throw std::invalid_argument("nonfinite scale");
    return {{{s.x,0,0},{0,s.y,0},{0,0,s.z}},{}};
}
[[nodiscard]] inline SceneAffine scene_rotate_y(double radians) {
    if(!std::isfinite(radians))throw std::invalid_argument("nonfinite angle");
    const double c=std::cos(radians),s=std::sin(radians);
    return {{{c,0,-s},{0,1,0},{s,0,c}},{}};
}
struct SceneNode {
    std::string name;
    std::optional<std::size_t> parent;
    std::optional<std::size_t> asset;
    SceneAffine local{};
};
struct SceneBounds {
    Point3 min{},max{};
};
struct SceneInstance {
    std::size_t node{},asset{};
    SceneAffine world{};
    SceneBounds bounds{};
};
struct SceneEvaluation {
    std::vector<SceneAffine> world;
    std::vector<std::size_t> order;
    std::vector<SceneInstance> instances;
};

class SceneForest {
public:
    std::vector<Mesh3> assets;
    std::vector<SceneNode> nodes;

    [[nodiscard]] SceneEvaluation evaluate() const {
        const auto n=nodes.size();
        if(n>10000||assets.size()>1000)
            throw std::invalid_argument("scene teaching budget exceeded");
        for(const auto& asset:assets) {
            asset.validate();
            if(asset.positions.empty())
                throw std::invalid_argument("empty shared mesh");
        }
        std::vector<std::vector<std::size_t>> children(n);
        std::vector<unsigned> indegree(n,0);
        std::queue<std::size_t> ready;
        for(std::size_t i=0;i<n;++i) {
            const auto& node=nodes[i];
            if(!finite(node.local.linear.x)||!finite(node.local.linear.y)||
               !finite(node.local.linear.z)||!finite(node.local.translation))
                throw std::invalid_argument("nonfinite node transform");
            if(node.asset&&*node.asset>=assets.size())
                throw std::out_of_range("invalid asset reference");
            if(node.parent) {
                if(*node.parent>=n)throw std::out_of_range("invalid parent");
                children[*node.parent].push_back(i);
                indegree[i]=1;
            } else ready.push(i);
        }
        SceneEvaluation result;
        result.world.resize(n);
        // Kahn's algorithm: parents precede children even if file order
        // is reversed; no recursion, so deeply nested graphs are safe.
        while(!ready.empty()) {
            const auto i=ready.front();ready.pop();
            const auto& node=nodes[i];
            result.world[i]=node.parent?
                result.world[*node.parent]*node.local:node.local;
            result.order.push_back(i);
            if(node.asset) {
                const auto& mesh=assets[*node.asset];
                const auto& w=result.world[i];
                SceneBounds b;
                b.min=b.max=w.point(mesh.positions.front());
                for(const auto p:mesh.positions) {
                    const auto q=w.point(p);
                    b.min={std::min(b.min.x,q.x),std::min(b.min.y,q.y),
                           std::min(b.min.z,q.z)};
                    b.max={std::max(b.max.x,q.x),std::max(b.max.y,q.y),
                           std::max(b.max.z,q.z)};
                }
                result.instances.push_back({i,*node.asset,w,b});
            }
            for(const auto j:children[i])if(--indegree[j]==0)ready.push(j);
        }
        if(result.order.size()!=n)
            throw std::invalid_argument("cycle in scene parent graph");
        return result;
    }

    // Reparent atomically, keeping either the local transform or world map.
    // Keeping the world pose requires an invertible new parent world map.
    // Full affine maps preserve shear, which a naive TRS decomposition may lose.
    void reparent(std::size_t child,std::optional<std::size_t> parent,
                  bool keep_world) {
        if(child>=nodes.size()||(parent&&*parent>=nodes.size()))
            throw std::out_of_range("reparent index");
        const auto before=evaluate();
        const auto old=nodes[child];
        SceneNode replacement=old;
        replacement.parent=parent;
        if(keep_world)
            replacement.local=parent?
                before.world[*parent].inverse()*before.world[child]:
                before.world[child];
        nodes[child]=std::move(replacement);
        try{(void)evaluate();}
        catch(...){nodes[child]=old;throw;}
    }
};

// One tree silhouette mesh: a single triangular crown and two trunk faces.
// The topology is local to the asset; instances never duplicate its storage.
[[nodiscard]] inline SceneForest five_tree_world() {
    SceneForest scene;
    Mesh3 tree;
    tree.positions={{-0.18,0,0},{0.18,0,0},{0.18,1,0},{-0.18,1,0},
                    {-0.8,0.8,0},{0.8,0.8,0},{0,2.5,0}};
    tree.triangles={{{0,1,2}},{{0,2,3}},{{4,5,6}}};
    scene.assets.push_back(std::move(tree));
    // Parent nodes do not need assets. Asset zero is referenced five times.
    scene.nodes={
        {"garden",{}, {},scene_translate({-3,0,0})},
        {"platform",{}, {},scene_translate({3,0,0})},
        {"tree A",0,0,scene_translate({0,0,0})},
        {"tree B",0,0,scene_translate({2,0,0})*scene_scale({0.7,0.7,0.7})},
        {"tree C",1,0,scene_translate({0,0,0})},
        {"tree D",1,0,scene_translate({2,0,0})*scene_scale({0.5,1.3,1})},
        {"tree E",{},0,scene_translate({0,0,-3})*scene_scale({-1,1,1})}
    };
    return scene;
}
} // namespace book03
