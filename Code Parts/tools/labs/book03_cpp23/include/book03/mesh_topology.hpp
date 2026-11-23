#pragma once

// Book 03 / Chapter 07 — indexed triangular mesh topology laboratory.
//
// Positions (geometry) and indices (topology) are NOT interchangeable.
// Two vertices can store identical XYZ doubles and yet be separate graph
// vertices, while two faces can share indices and have bad geometry.
//
// Deliberate contracts:
//   * Mesh3 stores indexed faces, optional PER-FACE-CORNER UV attributes.
//   * Indices are uint32_t and checked before indexing.
//   * EdgeKey sorts endpoint IDs, not coordinate values.
//   * EdgeUse preserves each face's directed traversal and original face ID.
//   * 1 incident face => boundary, 2 opposite => consistent interior,
//     2 same direction => winding conflict, >=3 => nonmanifold edge.
//   * A topologically repeated-index triangle is flagged and excluded from
//     normal edge-incidence analysis; a distinct-index collinear face is
//     flagged geometrically, but its indexed topology remains visible.
//   * Face connected components use shared EDGES, not shared points.
//   * A vertex can have multiple incident face fans despite every edge
//     having at most two incident faces (bow-tie vertex). Detect explicitly.
//   * Bitwise-distinct but equal-valued XYZ positions are reported as
//     duplicated-position *groups*. They are a diagnostic, not a reason to
//     automatically weld seams or damage artist-intended UV/normal splits.
//   * Euler V - E + F counts referenced vertices only, and is NOT in itself
//     a proof of watertightness, manifoldness, orientability or genus.
//
// Finite world coordinates are required. Geometrical zero-area detection
// uses an ordinary double cross product and can be imprecise for nearly
// degenerate triangles. The exact P05 oracle is only 2D. Topology counts
// and edge directions, by contrast, are exact discrete integer operations.
//
// This source is not a full half-edge library, exporter, mesh repairer,
// self-intersection test, sewing algorithm or a mesh simplifier.

#include "book03/geometry3d.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace book03 {

struct UV2 {
    double u{},v{};
};

struct TriangleIndices {
    std::array<std::uint32_t,3> vertex{};
};

struct Mesh3 {
    std::vector<Point3> positions{};
    std::vector<TriangleIndices> triangles{};
    // Each geometric face corner may have a different texture coordinate,
    // even when neighboring faces use the same position vertex ID.
    // Optional: empty means "no UV data"; otherwise exactly one array
    // per face. UVs are per corner, NOT forcibly stored per position.
    std::vector<std::array<UV2,3>> corner_uv{};

    void validate() const {
        if(positions.size()>std::numeric_limits<std::uint32_t>::max())
            throw std::out_of_range("too many indexed positions for uint32");
        for(Point3 p:positions) {
            if(!finite(p))
                throw std::invalid_argument("mesh positions must be finite");
        }
        for(const TriangleIndices& tri:triangles) {
            for(const auto id:tri.vertex) {
                if(id>=positions.size())
                    throw std::out_of_range("face vertex index out of bounds");
            }
        }
        if(!corner_uv.empty() && corner_uv.size()!=triangles.size())
            throw std::invalid_argument(
                "corner UV arrays must match face count");
        for(const auto& face:corner_uv) for(UV2 uv:face)
            if(!std::isfinite(uv.u)||!std::isfinite(uv.v))
                throw std::invalid_argument("mesh corner UV must be finite");
    }
};

struct EdgeKey {
    std::uint32_t lo{},hi{};
    [[nodiscard]] friend constexpr auto operator<=>(const EdgeKey&,
                                                       const EdgeKey&) = default;
};
[[nodiscard]] constexpr EdgeKey edge_key(std::uint32_t a,
                                         std::uint32_t b) noexcept {
    return {std::min(a,b),std::max(a,b)};
}

struct EdgeUse {
    std::size_t face{};
    std::uint32_t from{},to{};
};

enum class EdgeKind {
    Boundary,
    ConsistentInterior,
    WindingConflict,
    NonManifold
};

struct EdgeRecord {
    EdgeKey key{};
    std::vector<EdgeUse> uses{};
    EdgeKind kind{EdgeKind::Boundary};
};

struct MeshReport {
    std::size_t position_count{};
    std::size_t face_count{};
    std::size_t referenced_vertex_count{};
    std::size_t edge_count{};
    std::int64_t euler_characteristic{}; // Referenced V - E + all valid faces
    std::size_t boundary_edges{};
    std::size_t consistent_interior_edges{};
    std::size_t winding_conflict_edges{};
    std::size_t nonmanifold_edges{};
    std::size_t repeated_index_faces{};
    std::size_t zero_area_faces{};
    std::size_t unused_vertices{};
    std::size_t duplicate_position_groups{};
    std::size_t face_edge_components{};
    std::size_t disconnected_vertex_fans{};
    std::vector<std::size_t> repeated_face_ids{};
    std::vector<std::size_t> zero_area_face_ids{};
    std::vector<std::uint32_t> unused_vertex_ids{};
    std::vector<std::uint32_t> disconnected_fan_vertex_ids{};
    std::vector<EdgeRecord> edges{};
    std::vector<std::vector<std::uint32_t>> coincident_index_groups{};

    [[nodiscard]] bool closed_orientable_edge_candidate() const {
        // Necessary, not sufficient: no boundary/conflict/nonmanifold edge,
        // no degenerate face, no multi-fan vertex and nonempty face set.
        return face_count>0 && boundary_edges==0 &&
            winding_conflict_edges==0 && nonmanifold_edges==0 &&
            repeated_index_faces==0 && zero_area_faces==0 &&
            disconnected_vertex_fans==0;
    }
};

// A primitive DSU for graph-connected components across shared edges.
// It does not use floating-point vertex positions at all.
class MeshDisjointSet {
public:
    explicit MeshDisjointSet(std::size_t count)
        : parent_(count),rank_(count,0) {
        for(std::size_t i=0;i<count;++i) parent_[i]=i;
    }
    [[nodiscard]] std::size_t root(std::size_t x) {
        if(parent_[x]!=x) parent_[x]=root(parent_[x]);
        return parent_[x];
    }
    void join(std::size_t x,std::size_t y) {
        x=root(x);y=root(y);
        if(x==y) return;
        if(rank_[x]<rank_[y])std::swap(x,y);
        parent_[y]=x;
        if(rank_[x]==rank_[y])++rank_[x];
    }
private:
    std::vector<std::size_t> parent_;
    std::vector<std::uint8_t> rank_;
};

[[nodiscard]] inline MeshReport inspect_mesh(const Mesh3& mesh) {
    mesh.validate();
    MeshReport out{};
    out.position_count=mesh.positions.size();
    out.face_count=mesh.triangles.size();

    // Used vertices are based on all *validly indexed* source faces,
    // including degenerates, so "unused" means literally unreferenced.
    std::vector<bool> used(mesh.positions.size(),false);
    std::vector<bool> eligible(mesh.triangles.size(),true);
    std::map<EdgeKey,std::vector<EdgeUse>> incident;

    for(std::size_t f=0;f<mesh.triangles.size();++f) {
        const auto& ids=mesh.triangles[f].vertex;
        for(auto id:ids) used[id]=true;
        if(ids[0]==ids[1] || ids[1]==ids[2] ||
           ids[2]==ids[0]) {
            out.repeated_face_ids.push_back(f);
            eligible[f]=false;
            continue;
        }

        const Triangle3 tri{
            mesh.positions[ids[0]],
            mesh.positions[ids[1]],
            mesh.positions[ids[2]]};
        // Geometric degeneracy and topological validity are separate.
        // A geometrically degenerate face can still have 3 distinct ids.
        if(tri.doubled_area_vector()==Vector3{0,0,0})
            out.zero_area_face_ids.push_back(f);

        for(int corner=0;corner<3;++corner) {
            const auto a=ids[corner],b=ids[(corner+1)%3];
            incident[edge_key(a,b)].push_back({f,a,b});
        }
    }

    out.referenced_vertex_count=static_cast<std::size_t>(
        std::count(used.begin(),used.end(),true));
    for(std::size_t v=0;v<used.size();++v) if(!used[v])
        out.unused_vertex_ids.push_back(static_cast<std::uint32_t>(v));
    out.unused_vertices=out.unused_vertex_ids.size();
    out.repeated_index_faces=out.repeated_face_ids.size();
    out.zero_area_faces=out.zero_area_face_ids.size();

    // Index-based adjacency; all faces at a nonmanifold edge are connected
    // in the "share an edge" relation, but flagged as nonmanifold.
    MeshDisjointSet face_groups(mesh.triangles.size());
    for(const auto& [key,uses]:incident) {
        EdgeRecord edge{};
        edge.key=key;
        edge.uses=uses;
        if(uses.size()==1) {
            edge.kind=EdgeKind::Boundary;
            ++out.boundary_edges;
        } else if(uses.size()==2) {
            const bool opposite=uses[0].from==uses[1].to &&
                                uses[0].to==uses[1].from;
            edge.kind=opposite ?
                EdgeKind::ConsistentInterior:EdgeKind::WindingConflict;
            if(opposite) ++out.consistent_interior_edges;
            else ++out.winding_conflict_edges;
        } else {
            edge.kind=EdgeKind::NonManifold;
            ++out.nonmanifold_edges;
        }
        for(std::size_t u=1;u<uses.size();++u)
            face_groups.join(uses[0].face,uses[u].face);
        out.edges.push_back(std::move(edge));
    }

    out.edge_count=out.edges.size();
    // Euler characteristic is computed from non-repeated-index faces and
    // their edges, with only vertices actually belonging to those faces.
    // This avoids falsely treating excluded degenerate-index faces as cells.
    std::set<std::uint32_t> topo_vertices;
    std::size_t eligible_face_count=0;
    for(std::size_t f=0;f<mesh.triangles.size();++f) if(eligible[f]) {
        ++eligible_face_count;
        for(auto v:mesh.triangles[f].vertex) topo_vertices.insert(v);
    }
    out.euler_characteristic=
        static_cast<std::int64_t>(topo_vertices.size()) -
        static_cast<std::int64_t>(out.edge_count) +
        static_cast<std::int64_t>(eligible_face_count);

    // A repeated-index face is excluded from this adjacency graph:
    // it cannot contribute a normal well-defined 2D simplicial face.
    std::set<std::size_t> component_roots;
    for(std::size_t f=0;f<mesh.triangles.size();++f) if(eligible[f])
        component_roots.insert(face_groups.root(f));
    out.face_edge_components=component_roots.size();

    // Nonmanifold VERTEX fan test (bow tie): for each vertex, gather
    // incident eligible faces and connect them only across mesh edges
    // containing that vertex. A fan is connected iff there is one such
    // group. A manifold boundary vertex also has exactly two boundary
    // edges (or no boundaries for a closed vertex).
    for(std::size_t v=0;v<mesh.positions.size();++v) {
        if(!used[v]) continue;
        std::vector<std::size_t> vertex_faces;
        for(std::size_t f=0;f<mesh.triangles.size();++f) if(eligible[f]) {
            const auto& ids=mesh.triangles[f].vertex;
            if(ids[0]==v||ids[1]==v||ids[2]==v)
                vertex_faces.push_back(f);
        }
        if(vertex_faces.empty()) continue;
        std::map<std::size_t,std::size_t> face_local;
        for(std::size_t i=0;i<vertex_faces.size();++i)
            face_local.emplace(vertex_faces[i],i);
        MeshDisjointSet fans(vertex_faces.size());
        std::size_t local_boundary=0;
        for(const EdgeRecord& edge:out.edges) {
            if(edge.key.lo!=v && edge.key.hi!=v) continue;
            if(edge.kind==EdgeKind::Boundary) ++local_boundary;
            for(std::size_t k=1;k<edge.uses.size();++k)
                fans.join(face_local.at(edge.uses[0].face),
                          face_local.at(edge.uses[k].face));
        }
        std::set<std::size_t> distinct_fans;
        for(std::size_t i=0;i<vertex_faces.size();++i)
            distinct_fans.insert(fans.root(i));
        if(distinct_fans.size()!=1 ||
           (local_boundary!=0 && local_boundary!=2)) {
            out.disconnected_fan_vertex_ids.push_back(
                static_cast<std::uint32_t>(v));
        }
    }
    out.disconnected_vertex_fans=out.disconnected_fan_vertex_ids.size();

    // Exact equal-valued position groups, including +0 and -0 (both are
    // the same mathematical coordinate). This reports possible seams;
    // it DOES NOT collapse indices or infer whether welding is safe.
    std::map<std::tuple<double,double,double>,
             std::vector<std::uint32_t>> by_position;
    for(std::size_t i=0;i<mesh.positions.size();++i) {
        const Point3 q=mesh.positions[i];
        by_position[{q.x,q.y,q.z}].push_back(
            static_cast<std::uint32_t>(i));
    }
    for(auto& [coordinate,indices]:by_position) {
        (void)coordinate;
        if(indices.size()>1)
            out.coincident_index_groups.push_back(std::move(indices));
    }
    out.duplicate_position_groups=out.coincident_index_groups.size();
    return out;
}

// Familiar square in the 3D XY plane, consistently triangulated.
// V=4, E=5, F=2, chi=1, one connected disk, 4 boundary edges.
[[nodiscard]] inline Mesh3 reference_quad() {
    Mesh3 m{};
    m.positions={{0,0,0},{1,0,0},{1,1,0},{0,1,0}};
    m.triangles={{{0,1,2}},{{0,2,3}}};
    // Shared corners can have independent UVs. Adjacent faces need not
    // encode textures through a single per-position Vertex struct.
    m.corner_uv={{{{0,0},{1,0},{1,1}}},
                 {{{0,0},{1,1},{0,1}}}};
    return m;
}

// Closed, oriented genus-zero house, unlike an open wireframe.
// Floor (2 faces), walls (8), gables (2), sloped roof (4): 16 faces.
// Euler V-E+F = 2 is a *check*, not a proof of manifoldness.
[[nodiscard]] inline Mesh3 reference_house() {
    Mesh3 m{};
    m.positions={
        {-1.2,0,-1.2},{1.2,0,-1.2},{1.2,0,1.2},{-1.2,0,1.2},
        {-1.2,2,-1.2},{1.2,2,-1.2},{1.2,2,1.2},{-1.2,2,1.2},
        {0,3,-1.2},{0,3,1.2}
    };
    m.triangles={
        {{0,1,2}},{{0,2,3}},    // Downward floor
        {{3,2,6}},{{3,6,7}},    // Front wall, +Z
        {{0,5,1}},{{0,4,5}},    // Back wall, -Z
        {{0,3,7}},{{0,7,4}},    // Left wall, -X
        {{1,6,2}},{{1,5,6}},    // Right wall, +X
        {{7,6,9}},{{4,8,5}},    // Front and rear gables
        {{4,7,9}},{{4,9,8}},    // Left roof slope
        {{5,9,6}},{{5,8,9}}     // Right roof slope
    };
    return m;
}

[[nodiscard]] inline Mesh3 fixture_mesh(const std::string& name) {
    if(name=="quad") return reference_quad();
    if(name=="house") return reference_house();
    if(name=="flipped") {
        auto m=reference_quad();
        m.triangles[1].vertex={0,3,2};
        return m; // Shared (0,2) traversed twice in same direction.
    }
    if(name=="seam") {
        auto m=reference_quad();
        m.positions.insert(m.positions.end(),
            {m.positions[0],m.positions[2],m.positions[3]});
        m.triangles[1].vertex={4,5,6};
        return m; // Visually same two triangles, but two components.
    }
    if(name=="nonmanifold") {
        auto m=reference_quad();
        m.positions.push_back({0.5,-1,0});
        m.triangles.push_back({{0,2,4}});
        m.corner_uv.clear();
        return m; // Edge (0,2) has THREE incident faces.
    }
    if(name=="bowtie") {
        Mesh3 m{};
        m.positions={
            {0,0,0},{-1,0,0},{0,1,0},
            {1,0,0},{0,-1,0}
        };
        m.triangles={{{0,1,2}},{{0,3,4}}};
        return m; // Face fans touch at one vertex, but share no edge.
    }
    if(name=="degenerate") {
        auto m=reference_quad();
        m.triangles.push_back({{0,0,1}});
        m.corner_uv.clear();
        return m;
    }
    if(name=="collinear") {
        Mesh3 m{};
        m.positions={{0,0,0},{1,1,1},{2,2,2}};
        m.triangles={{{0,1,2}}};
        return m;
    }
    throw std::invalid_argument("unknown fixture name");
}

} // namespace book03
