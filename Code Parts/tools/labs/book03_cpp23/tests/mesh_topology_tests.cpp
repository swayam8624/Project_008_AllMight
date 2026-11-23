// Book 03 / P07 — deterministic topology and deliberately broken mesh cases.
//
// No expectations are derived from the inspector under test. Hand counts
// (V, E, F, boundary edges, Euler) and independent parity counting provide
// distinct oracles. Geometric equality does not imply index equality.
// This suite does not prove watertightness, self-intersection-free surfaces,
// genus, UV continuity, or correct materials.

#include "book03/mesh_topology.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using book03::Mesh3;
using book03::MeshReport;
using book03::EdgeKind;

void require(bool value,std::string_view why) {
    if(!value) throw std::runtime_error(std::string(why));
}
template<class E,class F> void must_throw(F&& fn,std::string_view why) {
    try { fn(); } catch(const E&) { return; }
    throw std::runtime_error(std::string(why));
}

std::size_t count_edges(const MeshReport& r,EdgeKind kind) {
    return static_cast<std::size_t>(std::count_if(
        r.edges.begin(),r.edges.end(),
        [&](const auto& item){return item.kind==kind;}));
}

void quad_is_a_disk() {
    const auto mesh=book03::reference_quad();
    const auto r=book03::inspect_mesh(mesh);
    require(r.position_count==4 && r.face_count==2,"quad source sizes");
    require(r.referenced_vertex_count==4 && r.edge_count==5,
            "quad has four referenced vertices, five undirected edges");
    require(r.euler_characteristic==1,"disk Euler characteristic one");
    require(r.boundary_edges==4 && r.consistent_interior_edges==1,
            "four boundary edges and one consistently wound diagonal");
    require(r.nonmanifold_edges==0 && r.winding_conflict_edges==0,
            "quad has no nonmanifold or winding conflict");
    require(r.face_edge_components==1 && r.disconnected_vertex_fans==0,
            "adjacent triangles form one connected face fan");
    require(!r.closed_orientable_edge_candidate(),
            "open quad must never be advertised as closed");
    require(count_edges(r,EdgeKind::Boundary)==4,
            "edge records agree with boundary counter");
    const auto common=book03::edge_key(0,2);
    const auto found=std::find_if(
        r.edges.begin(),r.edges.end(),
        [common](const auto& e){return e.key==common;});
    require(found!=r.edges.end() && found->uses.size()==2,
            "quad shares diagonal (0,2)");
    require(found->uses[0].from==2 && found->uses[0].to==0 &&
            found->uses[1].from==0 && found->uses[1].to==2,
            "shared edge is traversed in opposite directions");

    // Per-CORNER UVs exist independently of indexed position identity.
    require(mesh.corner_uv.size()==mesh.triangles.size(),"UV face count");
    require(mesh.corner_uv[0][0].u==0.0 && mesh.corner_uv[1][0].u==0.0,
            "per-face corner attributes are accessible");
}

void house_is_closed_oriented_candidate() {
    const auto mesh=book03::reference_house();
    const auto r=book03::inspect_mesh(mesh);
    require(r.position_count==10 && r.face_count==16,
            "house fixture is 10 positions and 16 triangular faces");
    require(r.referenced_vertex_count==10,"every house vertex is referenced");
    require(r.edge_count==24,"closed house edge count 24");
    require(r.euler_characteristic==2,
            "sphere-like genus-zero reference has Euler two");
    require(r.boundary_edges==0 &&
            r.consistent_interior_edges==24 &&
            r.winding_conflict_edges==0 && r.nonmanifold_edges==0,
            "each house edge belongs to two opposite traversals");
    require(r.face_edge_components==1 && r.disconnected_vertex_fans==0,
            "closed house is connected without a bow tie");
    require(r.duplicate_position_groups==0 &&
            r.repeated_index_faces==0 && r.zero_area_faces==0,
            "closed reference has no duplicate indices or degenerate faces");
    require(r.closed_orientable_edge_candidate(),
            "well-formed house meets necessary local closedness checks");
}

void flipped_winding_is_not_a_hole() {
    const auto r=book03::inspect_mesh(book03::fixture_mesh("flipped"));
    require(r.position_count==4 && r.face_count==2 && r.edge_count==5,
            "flipped square still has the same indexed topology counts");
    require(r.boundary_edges==4 && r.winding_conflict_edges==1,
            "shared diagonal is traversed in same direction");
    require(r.consistent_interior_edges==0 && r.nonmanifold_edges==0,
            "winding conflict is distinct from nonmanifold incidence");
    require(r.face_edge_components==1,"flipped triangles still share edge");
    require(r.euler_characteristic==1,
            "Euler characteristic alone cannot find winding conflict");
}

void seam_is_two_components_despite_same_pixels() {
    const auto m=book03::fixture_mesh("seam");
    const auto r=book03::inspect_mesh(m);
    require(r.position_count==7 && r.face_count==2,
            "seam duplicates three position entries");
    require(r.edge_count==6 && r.boundary_edges==6,
            "all edges are boundaries when index sets are disjoint");
    require(r.consistent_interior_edges==0 && r.face_edge_components==2,
            "visually touching triangles do not share indexed edges");
    require(r.duplicate_position_groups==3,
            "three exact equal-valued coordinates have distinct indices");
    require(r.euler_characteristic==2,
            "two disconnected triangles each contribute one to Euler");
    require(r.disconnected_vertex_fans==0,
            "seam creates separate vertices, not an indexed bow tie");
    require(m.positions[0]==m.positions[4] &&
            m.triangles[0].vertex[0]!=m.triangles[1].vertex[0],
            "geometry equality is not index identity");
}

void third_face_breaks_manifold_incidence() {
    const auto r=book03::inspect_mesh(
        book03::fixture_mesh("nonmanifold"));
    require(r.face_count==3 && r.nonmanifold_edges==1,
            "three incident faces on edge (0,2) is nonmanifold");
    require(count_edges(r,EdgeKind::NonManifold)==1,
            "edge record agrees with nonmanifold counter");
    require(r.face_edge_components==1,
            "all three triangles connected through offending edge");
    require(!r.closed_orientable_edge_candidate(),
            "three-face edge cannot be a closed two-manifold candidate");
    const auto common=book03::edge_key(0,2);
    const auto it=std::find_if(
        r.edges.begin(),r.edges.end(),
        [common](const auto& edge){return edge.key==common;});
    require(it!=r.edges.end() && it->uses.size()==3,
            "offending edge must preserve all three original face IDs");
}

void bow_tie_vertex_not_detected_by_edge_incidence_alone() {
    const auto r=book03::inspect_mesh(book03::fixture_mesh("bowtie"));
    require(r.position_count==5 && r.face_count==2,"bow-tie sizes");
    require(r.nonmanifold_edges==0 && r.winding_conflict_edges==0,
            "bow tie can have no edge with excess incidence");
    require(r.face_edge_components==2,
            "faces meeting only at one vertex are not edge-connected");
    require(r.disconnected_vertex_fans==1 &&
            r.disconnected_fan_vertex_ids==std::vector<std::uint32_t>{0},
            "shared center's incident face fans are disconnected");
    require(r.euler_characteristic==1,
            "Euler=1 alone does not imply one topological disk");
}

void distinct_degeneracies_are_separate() {
    const auto repeated=book03::inspect_mesh(
        book03::fixture_mesh("degenerate"));
    require(repeated.repeated_index_faces==1 &&
            repeated.repeated_face_ids==std::vector<std::size_t>{2},
            "repeated-index face flagged and excluded from edge graph");
    require(repeated.edge_count==5 && repeated.euler_characteristic==1,
            "excluded invalid face must not fabricate topological edges");

    const auto collinear=book03::inspect_mesh(
        book03::fixture_mesh("collinear"));
    require(collinear.repeated_index_faces==0 &&
            collinear.zero_area_faces==1 &&
            collinear.zero_area_face_ids==std::vector<std::size_t>{0},
            "three distinct indices can be geometrically collinear");
    require(collinear.edge_count==3 && collinear.boundary_edges==3,
            "geometry degeneracy does not erase indexed edges");
    require(!collinear.closed_orientable_edge_candidate(),
            "zero-area face fails strict geometry prerequisite");

    auto unused=book03::reference_quad();
    unused.positions.push_back({9,8,7});
    const auto report=book03::inspect_mesh(unused);
    require(report.unused_vertices==1 &&
            report.unused_vertex_ids==std::vector<std::uint32_t>{4},
            "unused position is not included in used Euler V");
    require(report.euler_characteristic==1,
            "unreferenced stored point does not change mesh-cell Euler");
}

void malformed_input_rejected() {
    const double nan=std::numeric_limits<double>::quiet_NaN();
    const double inf=std::numeric_limits<double>::infinity();
    auto out_of_range=book03::reference_quad();
    out_of_range.triangles[1].vertex[2]=77;
    must_throw<std::out_of_range>(
        [&]{(void)book03::inspect_mesh(out_of_range);},
        "out of bounds vertex ID must fail before indexing");
    auto nonfinite=book03::reference_quad();
    nonfinite.positions[1]={nan,0,0};
    must_throw<std::invalid_argument>(
        [&]{(void)book03::inspect_mesh(nonfinite);},
        "NaN vertex position must fail");
    auto infinite=book03::reference_quad();
    infinite.positions[0]={0,inf,0};
    must_throw<std::invalid_argument>(
        [&]{(void)book03::inspect_mesh(infinite);},
        "infinite vertex position must fail");
    auto missing_uv=book03::reference_quad();
    missing_uv.corner_uv.pop_back();
    must_throw<std::invalid_argument>(
        [&]{(void)book03::inspect_mesh(missing_uv);},
        "per-corner UV arrays must match face count if present");
    auto bad_uv=book03::reference_quad();
    bad_uv.corner_uv[0][0]={nan,0};
    must_throw<std::invalid_argument>(
        [&]{(void)book03::inspect_mesh(bad_uv);},
        "nonfinite UV data must be rejected");
    must_throw<std::invalid_argument>(
        []{(void)book03::fixture_mesh("neither-quad-nor-house");},
        "unknown fixture must be explicit error");
}

void seeded_independent_edge_count_witnesses() {
    // A strip of N quads, 2N consistently wound triangles, N+1 top and
    // bottom vertices each. This is a connected disk for all N>0:
    // V=2(N+1), F=2N, E=4N+1,
    // boundary E = 2N+2, interior E = 2N-1, Euler=1.
    // The formulas are derived from cell incidence, NOT inspect_mesh().
    std::uint32_t state=0xD1246B13u;
    for(int trial=0;trial<300;++trial) {
        state^=state<<13;state^=state>>17;state^=state<<5;
        const std::uint32_t n=1u+(state%50u);
        Mesh3 m{};
        for(std::uint32_t i=0;i<=n;++i) {
            m.positions.push_back({static_cast<double>(i),0,0});
            m.positions.push_back({static_cast<double>(i),1,0});
        }
        for(std::uint32_t i=0;i<n;++i) {
            const auto bottom=2*i, top=bottom+1,
                       next_bottom=bottom+2, next_top=bottom+3;
            // Counterclockwise in XY, positive Z normals for both.
            m.triangles.push_back({{bottom,next_bottom,next_top}});
            m.triangles.push_back({{bottom,next_top,top}});
        }
        const auto r=book03::inspect_mesh(m);
        require(r.position_count==2u*(n+1u),"strip positions");
        require(r.face_count==2u*n,"strip faces");
        require(r.edge_count==4u*n+1u,"strip independent edge count");
        require(r.boundary_edges==2u*n+2u,
                "strip independent boundary count");
        require(r.consistent_interior_edges==2u*n-1u,
                "strip internal edge count");
        require(r.face_edge_components==1&&r.disconnected_vertex_fans==0,
                "strip is one manifold disk");
        require(r.euler_characteristic==1,"strip Euler one");
    }
}

} // namespace

int main() {
    try {
        quad_is_a_disk();
        house_is_closed_oriented_candidate();
        flipped_winding_is_not_a_hole();
        seam_is_two_components_despite_same_pixels();
        third_face_breaks_manifold_incidence();
        bow_tie_vertex_not_detected_by_edge_incidence_alone();
        distinct_degeneracies_are_separate();
        malformed_input_rejected();
        seeded_independent_edge_count_witnesses();
        std::cout << "Book03/P07: topology contracts, closed house, "
                     "broken meshes, and 300 seeded strip oracles: PASS\n";
        return 0;
    } catch(const std::exception& err) {
        std::cerr<<"Book03/P07 FAILED: "<<err.what()<<'\n';
        return 1;
    }
}
