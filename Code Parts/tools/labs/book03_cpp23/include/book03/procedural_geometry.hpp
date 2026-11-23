#pragma once

// Book 03, Chapter 09 — real procedural geometry, standard C++23.
//
// A surface can be specified by:
//   1. explicit samples of a parametric map S(u,v);
//   2. an indexed mesh approximation with shared vertex identities;
//   3. a scalar field F(x,y,z) whose zero level set is the surface.
//
// We implement small, honest versions of each, without GPU, CAD or external
// geometry libraries. All lengths are in metres and +Y points upward.
// Every returned Mesh3 is validated and may be inspected by the independent
// Chapter 07 graph/topology module.
//
// Contracts and limits:
// - Heightfield is an OPEN topological disk; its analytic up-facing normal
//   has the form unit(-dh/dx, 1, -dh/dz). It is not a closed solid.
// - Cylinder with caps uses ONE shared indexed ring per boundary; its
//   geometric face normals form a sharp side/cap seam. A render pipeline
//   still needs separate per-corner normals to preserve that hard edge.
// - UV-like longitude parameters wrap by INTEGER indices, with no copy
//   of the 2*pi meridian. Sphere poles use ONE vertex each, not a ring
//   of zero-area coincident triangles. Neither construction automatically
//   supplies artist-authored UVs or displacement continuity.
// - The implicit-sphere extractor uses SIX consistent tetrahedra per
//   Cartesian grid cube, a STRICT sign convention F<0 inside, F>0 outside,
//   linear interpolation on crossing grid edges, and a global edge-key
//   cache for vertex identity. An exact F==0 sample is REJECTED rather
//   than silently duplicating or arbitrarily classifying a vertex.
// - Tetrahedral extraction approximates the zero-level surface of a
//   *piecewise linear interpolant of grid field samples*. It is not an
//   exact sphere, a certified SDF reconstruction or a general robust
//   marching-tetrahedra implementation for pathological input fields.
// - The isosurface class below is explicitly limited to a sphere centered
//   at the origin: outward triangle orientation is selected from radial
//   direction. General fields need their own reliable gradient/orientation
//   contracts and can contain multiple components or singularities.
// - Geometry uses double; signs on floating samples are not certified
//   under arbitrary near-zero cancellation. Topology uses exact integer
//   indices, edge keys and face traversal.
// - All generators bound resolution to limit memory and work; they refuse
//   invalid radii, zero steps, nonfinite arguments or grid exact-zero ties.

#include "book03/mesh_topology.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <numbers>
#include <stdexcept>
#include <utility>
#include <vector>

namespace book03 {

[[nodiscard]] inline double procedural_positive(
    double x,const char* reason) {
    if(!std::isfinite(x)||!(x>0.0)||x>1e4)
        throw std::invalid_argument(reason);
    return x;
}
[[nodiscard]] inline std::uint32_t procedural_resolution(
    std::uint32_t n,std::uint32_t low,std::uint32_t high) {
    if(n<low||n>high)
        throw std::invalid_argument("procedural resolution outside supported range");
    return n;
}

// A very ordinary but infinitely re-evaluable terrain function.
// y=h(x,z), h=amplitude*sin(frequency*x)*cos(frequency*z).
// Both spatial derivatives are analytic, not guessed by triangle normals.
struct TerrainField {
    double amplitude{0.45};
    double frequency{1.0};

    void validate() const {
        if(!std::isfinite(amplitude)||std::abs(amplitude)>1000.0||
           !std::isfinite(frequency)||!(frequency>0.0)||
           frequency>1000.0)
            throw std::invalid_argument("terrain amplitude/frequency invalid");
    }

    [[nodiscard]] double height(double x,double z) const {
        validate();
        if(!std::isfinite(x)||!std::isfinite(z))
            throw std::invalid_argument("terrain query position not finite");
        const double value=amplitude*
            std::sin(frequency*x)*std::cos(frequency*z);
        if(!std::isfinite(value))
            throw std::overflow_error("terrain height nonfinite");
        return value;
    }

    [[nodiscard]] Vector3 normal(double x,double z) const {
        validate();
        if(!std::isfinite(x)||!std::isfinite(z))
            throw std::invalid_argument("terrain normal query not finite");
        const double hx=amplitude*frequency*
            std::cos(frequency*x)*std::cos(frequency*z);
        const double hz=-amplitude*frequency*
            std::sin(frequency*x)*std::sin(frequency*z);
        return unit({-hx,1.0,-hz});
    }
};

[[nodiscard]] inline Mesh3 make_heightfield(
    const TerrainField& terrain,
    std::uint32_t x_cells,std::uint32_t z_cells,
    double x_width=4.0,double z_depth=4.0) {
    terrain.validate();
    procedural_resolution(x_cells,1,96);
    procedural_resolution(z_cells,1,96);
    procedural_positive(x_width,"terrain width must be positive");
    procedural_positive(z_depth,"terrain depth must be positive");
    Mesh3 mesh{};
    const std::size_t stride=static_cast<std::size_t>(z_cells)+1;
    const auto index=[stride](std::uint32_t i,std::uint32_t j) {
        return static_cast<std::uint32_t>(
            static_cast<std::size_t>(i)*stride+j);
    };
    for(std::uint32_t i=0;i<=x_cells;++i)
        for(std::uint32_t j=0;j<=z_cells;++j) {
            const double x=-x_width/2.0+
                x_width*static_cast<double>(i)/x_cells;
            const double z=-z_depth/2.0+
                z_depth*static_cast<double>(j)/z_cells;
            mesh.positions.push_back({x,terrain.height(x,z),z});
        }
    for(std::uint32_t i=0;i<x_cells;++i)
        for(std::uint32_t j=0;j<z_cells;++j) {
            const auto a=index(i,j),b=index(i+1,j),
                c=index(i+1,j+1),d=index(i,j+1);
            // X followed by Z is NEGATIVE Y. Reverse P08's XY order.
            mesh.triangles.push_back({{a,c,b}});
            mesh.triangles.push_back({{a,d,c}});
        }
    mesh.validate();
    return mesh;
}

// Analytic cylinder normals: radial for sides, ±Y for caps. A side-face
// corner and cap-face corner may have the SAME indexed position but must
// receive DIFFERENT shading normals when the edge is deliberately sharp.
[[nodiscard]] inline Vector3 cylinder_side_normal(double phi) {
    if(!std::isfinite(phi))
        throw std::invalid_argument("cylinder azimuth nonfinite");
    return {std::cos(phi),0.0,std::sin(phi)};
}

[[nodiscard]] inline Mesh3 make_capped_cylinder(
    std::uint32_t slices,double radius=1.0,double height=2.0) {
    procedural_resolution(slices,3,128);
    procedural_positive(radius,"cylinder radius must be positive");
    procedural_positive(height,"cylinder height must be positive");
    Mesh3 mesh{};
    const double tau=2.0*std::numbers::pi_v<double>;
    for(std::uint32_t layer=0;layer<2;++layer) {
        const double y=layer==0?0.0:height;
        for(std::uint32_t i=0;i<slices;++i) {
            const double phi=tau*static_cast<double>(i)/slices;
            mesh.positions.push_back({
                radius*std::cos(phi),y,radius*std::sin(phi)
            });
        }
    }
    const auto bottom=[&](std::uint32_t i)->std::uint32_t {
        return i%slices;
    };
    const auto top=[&](std::uint32_t i)->std::uint32_t {
        return slices+i%slices;
    };
    const auto bottom_center=static_cast<std::uint32_t>(mesh.positions.size());
    mesh.positions.push_back({0.0,0.0,0.0});
    const auto top_center=static_cast<std::uint32_t>(mesh.positions.size());
    mesh.positions.push_back({0.0,height,0.0});
    for(std::uint32_t i=0;i<slices;++i) {
        const auto a=bottom(i),b=bottom(i+1),
                   c=top(i),d=top(i+1);
        // +Y x +phi points outward in cylindrical XZ coordinates.
        mesh.triangles.push_back({{a,c,d}});
        mesh.triangles.push_back({{a,d,b}});
        // Bottom = -Y, top = +Y; caps reuse the ring IDs.
        mesh.triangles.push_back({{bottom_center,a,b}});
        mesh.triangles.push_back({{top_center,d,c}});
    }
    mesh.validate();
    return mesh;
}

// Latitude-longitude sphere. The north and south poles are single indexed
// vertices, not duplicated per slice. Only the INTERIOR rings have
// longitude samples. No final 2*pi duplicate is stored: modulo wraps.
[[nodiscard]] inline Mesh3 make_uv_sphere(
    std::uint32_t slices,std::uint32_t stacks,double radius=1.0) {
    procedural_resolution(slices,3,128);
    procedural_resolution(stacks,2,128);
    procedural_positive(radius,"sphere radius must be positive");
    Mesh3 mesh{};
    const double pi=std::numbers::pi_v<double>;
    mesh.positions.push_back({0.0,radius,0.0}); // north
    const auto ring=[slices](std::uint32_t j,std::uint32_t i) {
        return static_cast<std::uint32_t>(1+(j-1)*slices+i%slices);
    };
    for(std::uint32_t j=1;j<stacks;++j) {
        const double theta=pi*static_cast<double>(j)/stacks;
        for(std::uint32_t i=0;i<slices;++i) {
            const double phi=2.0*pi*static_cast<double>(i)/slices;
            const double sin_t=std::sin(theta);
            mesh.positions.push_back({
                radius*sin_t*std::cos(phi),
                radius*std::cos(theta),
                radius*sin_t*std::sin(phi)
            });
        }
    }
    const auto south=static_cast<std::uint32_t>(mesh.positions.size());
    mesh.positions.push_back({0.0,-radius,0.0});
    for(std::uint32_t i=0;i<slices;++i)
        mesh.triangles.push_back({{0,ring(1,i+1),ring(1,i)}});
    for(std::uint32_t j=1;j+1<stacks;++j)
        for(std::uint32_t i=0;i<slices;++i) {
            const auto a=ring(j,i),b=ring(j,i+1),
                       c=ring(j+1,i+1),d=ring(j+1,i);
            mesh.triangles.push_back({{a,b,c}});
            mesh.triangles.push_back({{a,c,d}});
        }
    for(std::uint32_t i=0;i<slices;++i)
        mesh.triangles.push_back({{
            ring(stacks-1,i),ring(stacks-1,i+1),south
        }});
    mesh.validate();
    return mesh;
}

// Sphere signed-distance: true Euclidean distance to the sphere, negative
// inside, positive outside; gradient is radial except at p=0 where the
// norm is not differentiable (and no unique normal exists).
struct SphereDistance {
    double radius{1.0};

    [[nodiscard]] double sample(Point3 p) const {
        procedural_positive(radius,"implicit sphere radius must be positive");
        if(!finite(p))
            throw std::invalid_argument("implicit query nonfinite");
        const double value=std::hypot(p.x,p.y,p.z)-radius;
        if(!std::isfinite(value))
            throw std::overflow_error("sphere field nonfinite");
        return value;
    }

    [[nodiscard]] Vector3 gradient(Point3 p) const {
        procedural_positive(radius,"implicit sphere radius must be positive");
        if(!finite(p))
            throw std::invalid_argument("implicit gradient query nonfinite");
        return unit({p.x,p.y,p.z});
    }
};

// Finite difference is useful for a numerical DIAGNOSTIC, but unlike the
// analytic sphere gradient its error depends on step and floating rounding.
// This is not an automatic guaranteed normal at cusps or field singularities.
[[nodiscard]] inline Vector3 central_gradient(
    const SphereDistance& field,Point3 p,double h) {
    procedural_positive(h,"finite-difference step must be positive");
    if(!finite(p))throw std::invalid_argument("nonfinite gradient query");
    const double inv=1.0/(2*h);
    const Vector3 g{
        (field.sample({p.x+h,p.y,p.z})-
         field.sample({p.x-h,p.y,p.z}))*inv,
        (field.sample({p.x,p.y+h,p.z})-
         field.sample({p.x,p.y-h,p.z}))*inv,
        (field.sample({p.x,p.y,p.z+h})-
         field.sample({p.x,p.y,p.z-h}))*inv
    };
    if(!finite(g)) throw std::overflow_error("finite difference overflow");
    return g;
}

// Edge keys use LINEAR GRID-VERTEX IDs, not positions. The cache ensures
// that two tetrahedra intersecting the same grid edge reference the SAME
// resulting mesh vertex. Orientation below is intentionally sphere-specific.
struct SphereIsoMesh {
    Mesh3 mesh{};
    std::size_t lattice_samples{};
    std::size_t cached_crossings{};
    std::size_t occupied_lattice_samples{};
};

[[nodiscard]] inline SphereIsoMesh extract_sphere_isosurface(
    const SphereDistance& field,
    std::uint32_t cells,double extent) {
    procedural_resolution(cells,3,24);
    procedural_positive(extent,"grid half-extent must be positive");
    procedural_positive(field.radius,"implicit sphere radius invalid");
    if(!(field.radius<extent))
        throw std::invalid_argument(
            "sphere must fit entirely inside grid half-extent");

    const std::uint32_t n=cells+1;
    const std::size_t lattice_size=static_cast<std::size_t>(n)*n*n;
    const auto lattice_id=[n](std::uint32_t x,
                               std::uint32_t y,
                               std::uint32_t z)->std::uint32_t {
        return (x*n+y)*n+z;
    };
    struct Sample {Point3 p{};double f{};};
    std::vector<Sample> lattice(lattice_size);
    std::size_t interior=0,exterior=0;
    for(std::uint32_t i=0;i<n;++i)
        for(std::uint32_t j=0;j<n;++j)
            for(std::uint32_t k=0;k<n;++k) {
                const auto id=lattice_id(i,j,k);
                const Point3 p{
                    -extent+2*extent*static_cast<double>(i)/cells,
                    -extent+2*extent*static_cast<double>(j)/cells,
                    -extent+2*extent*static_cast<double>(k)/cells
                };
                const double value=field.sample(p);
                // A grid vertex precisely on the requested isovalue has
                // ambiguous intersection ownership for this simple lab.
                // Make it explicit rather than create zero-area triangles.
                if(value==0.0)
                    throw std::invalid_argument(
                        "grid sample exactly on isosurface: change resolution or extent");
                lattice[id]={p,value};
                if(value<0)++interior;else ++exterior;
            }
    if(!interior||!exterior)
        throw std::invalid_argument(
            "no sampled inside/outside transition; refine grid or radius");

    SphereIsoMesh out{};
    out.lattice_samples=lattice_size;
    out.occupied_lattice_samples=interior;
    std::map<EdgeKey,std::uint32_t> edge_vertex{};
    const auto cross_vertex=[&](std::uint32_t a,
                                 std::uint32_t b)->std::uint32_t {
        const EdgeKey key=edge_key(a,b);
        if(const auto it=edge_vertex.find(key);it!=edge_vertex.end())
            return it->second;
        const auto& A=lattice[a];
        const auto& B=lattice[b];
        if((A.f<0)==(B.f<0))
            throw std::logic_error("iso intersection requires opposite signs");
        // This linear interpolation is on the sampled scalar field, not the
        // true analytic sphere. Its approximation is explicit.
        const double t=A.f/(A.f-B.f);
        if(!std::isfinite(t)||!(t>0.0)||!(t<1.0))
            throw std::runtime_error("unstable isosurface edge interpolation");
        const Point3 p=A.p+t*(B.p-A.p);
        if(!finite(p))
            throw std::overflow_error("isosurface vertex nonfinite");
        if(out.mesh.positions.size()>=std::numeric_limits<std::uint32_t>::max())
            throw std::overflow_error("isosurface vertex count overflow");
        const auto idx=static_cast<std::uint32_t>(
            out.mesh.positions.size());
        out.mesh.positions.push_back(p);
        edge_vertex.emplace(key,idx);
        return idx;
    };

    const auto add_triangle=[&](std::uint32_t a,
                                 std::uint32_t b,
                                 std::uint32_t c) {
        if(a==b||b==c||c==a)
            throw std::logic_error("repeated iso triangle index");
        const Point3 A=out.mesh.positions[a],B=out.mesh.positions[b],
                     C=out.mesh.positions[c];
        const Vector3 normal=cross(B-A,C-A);
        const Point3 center{
            (A.x+B.x+C.x)/3.0,
            (A.y+B.y+C.y)/3.0,
            (A.z+B.z+C.z)/3.0
        };
        const double alignment=dot(normal,
             Vector3{center.x,center.y,center.z});
        if(alignment==0.0)
            throw std::runtime_error("degenerate implicit sphere triangle");
        if(alignment<0.0)std::swap(b,c);
        out.mesh.triangles.push_back({{a,b,c}});
    };

    // Six tetrahedral cells around the same cube diagonal (000 -> 111).
    // The choice is globally fixed so face triangulations agree across
    // neighboring Cartesian cells. The 8 local cube corner IDs are:
    // 000 100 110 010 001 101 111 011.
    constexpr std::array<std::array<int,3>,8> corners{{
        {{0,0,0}},{{1,0,0}},{{1,1,0}},{{0,1,0}},
        {{0,0,1}},{{1,0,1}},{{1,1,1}},{{0,1,1}}
    }};
    constexpr std::array<std::array<int,4>,6> tets{{
        {{0,1,2,6}},{{0,2,3,6}},{{0,3,7,6}},
        {{0,7,4,6}},{{0,4,5,6}},{{0,5,1,6}}
    }};
    for(std::uint32_t i=0;i<cells;++i)
        for(std::uint32_t j=0;j<cells;++j)
            for(std::uint32_t k=0;k<cells;++k) {
                std::array<std::uint32_t,8> cube_ids{};
                for(int corner=0;corner<8;++corner)
                    cube_ids[corner]=lattice_id(
                        i+corners[corner][0],
                        j+corners[corner][1],
                        k+corners[corner][2]);
                for(const auto& tet:tets) {
                    std::array<std::uint32_t,4> in{},outside{};
                    std::size_t ni=0,no=0;
                    for(int a:tet) {
                        const auto id=cube_ids[a];
                        if(lattice[id].f<0.0)in[ni++]=id;
                        else outside[no++]=id;
                    }
                    if(ni==0||ni==4)continue;
                    if(ni==1||ni==3) {
                        std::array<std::uint32_t,3> q{};
                        if(ni==1)
                            for(int m=0;m<3;++m)
                                q[m]=cross_vertex(in[0],outside[m]);
                        else
                            for(int m=0;m<3;++m)
                                q[m]=cross_vertex(outside[0],in[m]);
                        add_triangle(q[0],q[1],q[2]);
                    } else { // ni==2: quadrilateral, split consistently.
                        const auto a=cross_vertex(in[0],outside[0]);
                        const auto b=cross_vertex(in[0],outside[1]);
                        const auto c=cross_vertex(in[1],outside[1]);
                        const auto d=cross_vertex(in[1],outside[0]);
                        add_triangle(a,b,c);
                        add_triangle(a,c,d);
                    }
                }
            }
    out.cached_crossings=edge_vertex.size();
    if(out.mesh.triangles.empty())
        throw std::runtime_error("no isosurface triangles extracted");
    out.mesh.validate();
    return out;
}

} // namespace book03
