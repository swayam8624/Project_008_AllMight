// Book03/P09 — procedural surface and implicit sphere correctness tests.
//
// These use independent closed-form geometry and combinatorial counts.
// A topology pass does not certify geometric embedding, but bad mesh seams,
// repeated poles, wrong winding and duplicated indexing are caught here.
// The implicit-meshing oracle is intentionally a sphere-specific smoke test;
// its exact numeric vertex/face count is resolution dependent, so we check
// Euler, closure, signed-side behavior and shared indexed edge incidence.

#include "book03/procedural_geometry.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

using book03::Point3;
using book03::Vector3;
using book03::Mesh3;
using book03::MeshReport;

void require(bool good,std::string_view why) {
    if(!good)throw std::runtime_error(std::string(why));
}
void near(double a,double b,double eps,std::string_view why) {
    if(!std::isfinite(a)||!std::isfinite(b)||
       std::abs(a-b)>eps*(1.0+std::abs(b)))
        throw std::runtime_error(std::string(why));
}
void near_vec(Vector3 a,Vector3 b,double eps,std::string_view why) {
    near(a.x,b.x,eps,why);near(a.y,b.y,eps,why);near(a.z,b.z,eps,why);
}
template<class Ex,class F>
void rejected(F&& fn,std::string_view why) {
    try{fn();}
    catch(const Ex&){return;}
    throw std::runtime_error(std::string(why));
}

void good_open_disk(const MeshReport& r,
                    std::size_t v,std::size_t e,
                    std::size_t f,std::size_t boundary) {
    require(r.position_count==v &&
            r.referenced_vertex_count==v &&
            r.edge_count==e &&
            r.face_count==f &&
            r.boundary_edges==boundary &&
            r.consistent_interior_edges==e-boundary &&
            r.euler_characteristic==1 &&
            r.face_edge_components==1 &&
            r.disconnected_vertex_fans==0 &&
            r.winding_conflict_edges==0 &&
            r.nonmanifold_edges==0 &&
            r.repeated_index_faces==0 &&
            r.zero_area_faces==0,
            "open indexed grid must be a consistently wound disk");
}
void good_closed_sphere(const MeshReport& r,
                        std::size_t v,std::size_t e,std::size_t f) {
    require(r.position_count==v &&
            r.referenced_vertex_count==v &&
            r.edge_count==e &&
            r.face_count==f &&
            r.boundary_edges==0 &&
            r.consistent_interior_edges==e &&
            r.euler_characteristic==2 &&
            r.face_edge_components==1 &&
            r.disconnected_vertex_fans==0 &&
            r.winding_conflict_edges==0 &&
            r.nonmanifold_edges==0 &&
            r.repeated_index_faces==0 &&
            r.zero_area_faces==0 &&
            r.closed_orientable_edge_candidate(),
            "closed single-shell surface must be indexed sphere candidate");
}

void analytic_terrain_field_and_grid_counts() {
    const book03::TerrainField field{0.4,1.0};
    near(field.height(0,0),0,0,"terrain origin height exactly zero");
    near(field.height(std::numbers::pi/2,0),0.4,1e-14,
         "terrain peak height");
    near_vec(field.normal(0,0),
             book03::unit({-0.4,1,0}),1e-14,
             "terrain analytic gradient at origin");
    const Point3 location{0.3,0,0.4};
    const double h=1e-5;
    const Vector3 finite_slope{
        (field.height(location.x+h,location.z)-
         field.height(location.x-h,location.z))/(2*h),
        1.0,
        (field.height(location.x,location.z+h)-
         field.height(location.x,location.z-h))/(2*h)
    };
    near_vec(field.normal(location.x,location.z),
             book03::unit({-finite_slope.x,1,-finite_slope.z}),
             2e-10,"analytic terrain normal vs independent finite difference");

    const Mesh3 mesh=book03::make_heightfield(field,8,10,4,4);
    const auto r=book03::inspect_mesh(mesh);
    good_open_disk(r,99,258,160,36);
    for(const auto& face:mesh.triangles) {
        const auto& idx=face.vertex;
        const book03::Triangle3 t{
            mesh.positions[idx[0]],
            mesh.positions[idx[1]],
            mesh.positions[idx[2]]
        };
        require(t.doubled_area_vector().y>0,
                "XZ terrain must point upward despite X cross Z = -Y");
    }
    const auto tiny=book03::make_heightfield(field,1,1,2,2);
    good_open_disk(book03::inspect_mesh(tiny),4,5,2,4);

    // Scaling terrain domain uniformly changes positions and
    // derivatives, not the index structure of this fixed grid.
    const auto different=book03::make_heightfield(field,8,10,6,9);
    good_open_disk(book03::inspect_mesh(different),99,258,160,36);
}

void capped_cylinder_indices_and_sharp_normal_boundary() {
    constexpr std::uint32_t n=12;
    const auto mesh=book03::make_capped_cylinder(n,1.25,2.5);
    const auto r=book03::inspect_mesh(mesh);
    good_closed_sphere(r,2*n+2,6*n,4*n);
    require(mesh.positions.size()==26 &&
            mesh.triangles.size()==48,
            "one cap center at each end and one ring per height");
    const auto half=book03::make_capped_cylinder(3,1,1);
    good_closed_sphere(book03::inspect_mesh(half),8,18,12);
    for(const auto& f:mesh.triangles) {
        const auto& ids=f.vertex;
        const auto a=mesh.positions[ids[0]];
        const auto b=mesh.positions[ids[1]];
        const auto c=mesh.positions[ids[2]];
        const auto cross=(book03::cross(b-a,c-a));
        const Point3 ctr{(a.x+b.x+c.x)/3,
                         (a.y+b.y+c.y)/3,
                         (a.z+b.z+c.z)/3};
        // Side tri normals are radial; cap normals vertical.
        if(ctr.y>1e-9&&ctr.y<2.5-1e-9)
            require(cross.x*ctr.x+cross.z*ctr.z>0,
                    "side triangles point away from cylinder axis");
        else if(ctr.y==0)
            require(cross.y<0,"bottom cap faces outward -Y");
        else if(ctr.y==2.5)
            require(cross.y>0,"top cap faces outward +Y");
    }
    near_vec(book03::cylinder_side_normal(0),{1,0,0},0,
             "analytic cylinder side normal");
    near_vec(book03::cylinder_side_normal(std::numbers::pi/2),
             {0,0,1},1e-14,"analytic cylinder quarter-turn normal");
    require(book03::cylinder_side_normal(0)!=Vector3{0,-1,0},
            "side and cap have different shading normals at same position");
}

void sphere_poles_wrap_and_orientation() {
    for(const auto [slices,stacks]:
        std::vector<std::pair<std::uint32_t,std::uint32_t>>{
            {3,2},{7,4},{12,6},{23,11}
        }) {
        const auto mesh=book03::make_uv_sphere(slices,stacks,2);
        const auto rings=stacks-1;
        const std::size_t v=2u+slices*rings;
        const std::size_t f=2u*slices*rings;
        const std::size_t e=3u*slices*rings;
        good_closed_sphere(book03::inspect_mesh(mesh),v,e,f);
        require(mesh.positions.front()==Point3{0,2,0} &&
                mesh.positions.back()==Point3{0,-2,0},
                "one indexed pole per end, no duplicated meridian vertex");
        for(const auto& t:mesh.triangles) {
            const auto a=mesh.positions[t.vertex[0]];
            const auto b=mesh.positions[t.vertex[1]];
            const auto c=mesh.positions[t.vertex[2]];
            const Vector3 geometric=book03::cross(b-a,c-a);
            const Vector3 radial{
                (a.x+b.x+c.x)/3,
                (a.y+b.y+c.y)/3,
                (a.z+b.z+c.z)/3
            };
            require(book03::dot(geometric,radial)>0.0,
                    "each triangulated sphere face must point outward");
        }
        for(Point3 p:mesh.positions)
            near(std::hypot(p.x,p.y,p.z),2,1e-13,
                 "all analytic sphere vertices have radius two");
    }
}

void signed_distance_sign_gradient_and_finite_difference() {
    const book03::SphereDistance field{1.25};
    near(field.sample({0,0,0}),-1.25,0,
         "origin inside sphere with negative signed distance");
    near(field.sample({1.25,0,0}),0,0,
         "point on sphere is zero level set");
    near(field.sample({0,0,-2.25}),1,0,
         "outside is positive distance");
    near_vec(field.gradient({0,3,0}),{0,1,0},0,
             "gradient points outward");
    const Point3 p{1.6,0.7,0.3};
    const Vector3 finite=book03::central_gradient(field,p,1e-5);
    near_vec(book03::unit(finite),field.gradient(p),2e-10,
             "central difference approximates analytic sphere gradient");
    rejected<std::invalid_argument>(
        [&]{(void)field.gradient({0,0,0});},
        "SDF gradient at sphere center undefined; must reject");
    rejected<std::invalid_argument>(
        [&]{(void)book03::central_gradient(field,p,0);},
        "zero finite-difference step invalid");
    const book03::TerrainField constant{0.0,1.0};
    near_vec(constant.normal(0,0),{0,1,0},0,
             "flat terrain still has up normal");
}

void marching_tetra_sphere_shared_index_and_shape() {
    const book03::SphereDistance field{0.91};
    for(const std::uint32_t n:{9u,13u}) {
        const auto result=book03::extract_sphere_isosurface(field,n,1.5);
        require(result.lattice_samples==
                static_cast<std::size_t>(n+1)*(n+1)*(n+1),
                "lattice sample count is an integer cube");
        require(result.cached_crossings==result.mesh.positions.size(),
                "one unique vertex per intersected global grid edge");
        require(result.occupied_lattice_samples>0,
                "extracted sphere must contain negative sampled values");
        const auto report=book03::inspect_mesh(result.mesh);
        require(report.position_count>16 && report.face_count>25,
                "extracted surface cannot be trivial");
        // A closed genus-zero sphere is expected when entirely inside grid,
        // with all tetra face triangulations consistent and properly cached.
        good_closed_sphere(report,report.position_count,
                           report.edge_count,report.face_count);
        require(report.face_count==2*report.position_count-4 &&
                report.edge_count==3*report.position_count-6,
                "triangulated connected closed genus-zero sphere identity");
        for(const auto& p:result.mesh.positions) {
            const double radius=std::hypot(p.x,p.y,p.z);
            require(radius>0.0 && radius<field.radius+1e-10,
                    "linear field interpolation sphere vertex inside or on true sphere");
        }
        for(const auto& tri:result.mesh.triangles) {
            const Point3 a=result.mesh.positions[tri.vertex[0]];
            const Point3 b=result.mesh.positions[tri.vertex[1]];
            const Point3 c=result.mesh.positions[tri.vertex[2]];
            const Vector3 nrm=book03::cross(b-a,c-a);
            const Vector3 radial{
                (a.x+b.x+c.x)/3,
                (a.y+b.y+c.y)/3,
                (a.z+b.z+c.z)/3
            };
            require(book03::dot(nrm,radial)>0,
                    "each implicit sphere triangle wound outward");
        }
    }
}

void strict_rejection_of_invalid_parameters_and_ties() {
    const book03::TerrainField t{0.4,1.0};
    const auto nan=std::numeric_limits<double>::quiet_NaN();
    const auto inf=std::numeric_limits<double>::infinity();
    rejected<std::invalid_argument>(
        [&]{(void)book03::make_heightfield(t,0,4);},
        "zero heightfield resolution must reject");
    rejected<std::invalid_argument>(
        [&]{(void)book03::make_heightfield(t,200,4);},
        "too large heightfield resolution must reject");
    rejected<std::invalid_argument>(
        [&]{(void)book03::make_heightfield(t,4,4,nan,1);},
        "nonfinite terrain extent reject");
    rejected<std::invalid_argument>(
        [&]{(void)book03::make_capped_cylinder(2,1,2);},
        "polygon cylinder needs at least three angular slices");
    rejected<std::invalid_argument>(
        [&]{(void)book03::make_capped_cylinder(8,0,2);},
        "zero cylinder radius reject");
    rejected<std::invalid_argument>(
        [&]{(void)book03::make_uv_sphere(6,1);},
        "sphere requires at least two latitude stacks");
    rejected<std::invalid_argument>(
        [&]{(void)book03::make_uv_sphere(6,3,inf);},
        "sphere infinite radius reject");
    rejected<std::invalid_argument>(
        [&]{(void)book03::SphereDistance{0.0}.sample({1,0,0});},
        "invalid implicit radius reject");
    rejected<std::invalid_argument>(
        [&]{(void)book03::extract_sphere_isosurface({1.0},2,2.0);},
        "grid below bounded resolution reject");
    rejected<std::invalid_argument>(
        [&]{(void)book03::extract_sphere_isosurface({2.0},9,1.0);},
        "sphere touching grid bounding box reject");
    rejected<std::invalid_argument>(
        [&]{(void)book03::extract_sphere_isosurface({0.001},3,1.5);},
        "tiny sphere with no inside lattice samples must reject");
    // n=4, extent=2 -> grid x∈{-2,-1,0,1,2}. r=1 passes exactly
    // through multiple grid vertices. The restricted extractor will reject
    // these ties instead of silently fabricating degenerate triangles.
    rejected<std::invalid_argument>(
        [&]{(void)book03::extract_sphere_isosurface({1.0},4,2.0);},
        "exact zero grid values explicitly require another grid choice");
}

} // namespace

int main() {
    try {
        analytic_terrain_field_and_grid_counts();
        capped_cylinder_indices_and_sharp_normal_boundary();
        sphere_poles_wrap_and_orientation();
        signed_distance_sign_gradient_and_finite_difference();
        marching_tetra_sphere_shared_index_and_shape();
        strict_rejection_of_invalid_parameters_and_ties();
        std::cout<<"Book03/P09: procedural terrain, capped cylinder, "
                    "sphere poles and cached-edge implicit isosurface: PASS\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"Book03/P09 FAILED: "<<error.what()<<'\n';
        return 1;
    }
}
