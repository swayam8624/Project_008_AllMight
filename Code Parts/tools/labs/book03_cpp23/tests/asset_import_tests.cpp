// Book03/P10 — independently check OBJ index-domain preservation,
// rejection behavior, convex-quad triangulation and glTF bounds math.
//
// We test "file parsed" independently from "topology is trustworthy".
// A badly wound or degenerate but syntactically legal triangle is not
// silently discarded: P07 reports why downstream operations may reject it.
// This importer supports an explicit OBJ subset, not all OBJ or glTF 2.0.

#include "book03/asset_import.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

void require(bool pass,std::string_view reason) {
    if(!pass)throw std::runtime_error(std::string(reason));
}
template<class Ex,class F>
void rejects(F&& action,std::string_view why) {
    try{action();}
    catch(const Ex&){return;}
    throw std::runtime_error(std::string(why));
}
[[nodiscard]] book03::ObjAsset parse(const std::string& text,
                                book03::ObjImportLimits limits={}) {
    std::istringstream input{text};
    return book03::import_obj(input,limits);
}

constexpr std::string_view square_seam=
    "# two triangles share geometric IDs but have hard UV and normal seams\n"
    "v 0 0 0\n"
    "v 1 0 0\n"
    "v 1 1 0\n"
    "v 0 1 0\n"
    "vt 0 0\n"
    "vt 1 0\n"
    "vt 1 1\n"
    "vt 0.2 0\n"
    "vt 0.9 0.9\n"
    "vt 0.2 1\n"
    "vn 0 0 1\n"
    "vn 0 0 -1\n"
    "f 1/1/1 2/2/1 3/3/1\n"
    "f 1/4/2 3/5/2 4/6/2\n";

void separate_index_domains_and_topological_identity() {
    const auto asset=parse(std::string(square_seam));
    const auto topo=asset.topological_mesh();
    const auto report=book03::inspect_mesh(topo);
    require(asset.positions.size()==4 && asset.texcoords.size()==6 &&
            asset.normals.size()==2 && asset.faces.size()==2,
            "OBJ independent position/UV/normal streams have distinct counts");
    require(report.position_count==4&&report.edge_count==5&&
            report.face_count==2&&report.boundary_edges==4&&
            report.consistent_interior_edges==1&&
            report.euler_characteristic==1&&
            report.face_edge_components==1,
            "separate UV/normal seams do not sever shared geometric quad");
    require(asset.faces[0].corners[0].position==
            asset.faces[1].corners[0].position &&
            asset.faces[0].corners[0].texcoord!=
            asset.faces[1].corners[0].texcoord &&
            asset.faces[0].corners[0].normal!=
            asset.faces[1].corners[0].normal,
            "same geometric vertex can carry different face-corner attributes");
    const auto rendering=asset.to_rendering_mesh();
    require(rendering.vertices.size()==6 &&
            rendering.triangles.size()==2,
            "GPU tuple stream splits UV and normal seam to six vertices");
    require(rendering.vertices[0].uv &&
            rendering.vertices[0].normal &&
            rendering.vertices[3].uv &&
            rendering.vertices[3].normal,
            "GPU tuples preserve independently selected UVs and normals");
    const auto info=book03::inspect_obj_asset(asset);
    require(info.geometric_vertices==4&&info.rendering_vertices==6&&
            info.triangles==2&&info.invalid_supplied_normals==0&&
            info.warnings==0,
            "asset audit explicitly reports topological and render IDs");
    require(info.bounds_min==book03::Point3{0,0,0}&&
            info.bounds_max==book03::Point3{1,1,0},
            "AABB uses exact finite source positions");
}

void independent_negative_indices_and_face_time_scopes() {
    const auto a=parse(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "vt 0 0\n"
        "vt 1 0\n"
        "vt 0 1\n"
        "vn 0 0 1\n"
        "f -3/-3/-1 -2/-2/-1 -1/-1/-1\n"
        "v 2 2 2\n" // later vertex must NOT change earlier -1
    );
    require(a.faces.size()==1 &&
            a.faces[0].corners[0].position==0 &&
            a.faces[0].corners[1].position==1 &&
            a.faces[0].corners[2].position==2,
            "negative indices resolved at FACE time");
    require(*a.faces[0].corners[2].texcoord==2 &&
            *a.faces[0].corners[0].normal==0,
            "negative UV and normal offsets use their OWN stream sizes");
    const auto report=book03::inspect_mesh(a.topological_mesh());
    require(report.unused_vertices==1,
            "later position remains unused by prior resolved face");
    const auto b=parse(
        "v 0 0 0\nv 1 0 0\nv 0 1 0\n"
        "f 1 2 3\n"
    );
    for(const auto& c:b.faces[0].corners)
        require(!c.texcoord && !c.normal,
                "no UV or normal invented for position-only OBJ");
    const auto c=parse(
        "v 0 0 0\nv 1 0 0\nv 0 1 0\n"
        "vn 0 0 1\nf 1//1 2//1 3//1\n"
    );
    for(const auto& k:c.faces[0].corners)
        require(!k.texcoord && k.normal &&
                *k.normal==0,
                "OBJ p//n parsing preserves absent UV and present normal");
}

void convex_quad_triangulation_is_explicitly_restricted() {
    const auto a=parse(
        "v 0 0 0\nv 2 0 0\nv 2 2 0\nv 0 2 0\n"
        "f 1 2 3 4\n"
    );
    require(a.faces.size()==2 &&
            a.faces[0].corners[0].position==0 &&
            a.faces[0].corners[1].position==1 &&
            a.faces[0].corners[2].position==2 &&
            a.faces[1].corners[0].position==0 &&
            a.faces[1].corners[1].position==2 &&
            a.faces[1].corners[2].position==3,
            "a planar convex quad is explicitly triangulated as 0,1,2 and 0,2,3");
    const auto r=book03::inspect_mesh(a.topological_mesh());
    require(r.euler_characteristic==1&&r.edge_count==5&&
            r.consistent_interior_edges==1,
            "quad fan yields the same indexed disk as P07 reference");

    rejects<book03::ObjInputError>([&]{(void)parse(
        "v 0 0 0\nv 2 0 0\nv 0.4 0.4 0\nv 0 2 0\n"
        "f 1 2 3 4\n");},
        "a concave quad is rejected rather than fan-triangulated incorrectly");
    rejects<book03::ObjInputError>([&]{(void)parse(
        "v 0 0 0\nv 1 1 0\nv 0 1 0\nv 1 0 0\n"
        "f 1 2 3 4\n");},
        "a self-intersecting quad is rejected");
    rejects<book03::ObjInputError>([&]{(void)parse(
        "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0.5\n"
        "f 1 2 3 4\n");},
        "a nonplanar quad is rejected");
    rejects<book03::ObjInputError>([&]{(void)parse(
        "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nv -1 0 0\n"
        "f 1 2 3 4 5\n");},
        "unsupported general OBJ N-gon cannot silently use fan triangulation");
}

void parsing_does_not_imply_geometric_or_topological_validity() {
    const auto repeated=parse(
        "v 0 0 0\nv 1 0 0\nv 0 1 0\n"
        "f 1 2 2\n"
    );
    const auto repeated_report=book03::inspect_obj_asset(repeated);
    require(repeated_report.topology.repeated_index_faces==1,
            "syntactically valid repeated-index face remains visible to audit");
    const auto zero_area=parse(
        "v 0 0 0\nv 1 0 0\nv 2 0 0\n"
        "f 1 2 3\n"
    );
    const auto zero_report=book03::inspect_obj_asset(zero_area);
    require(zero_report.topology.zero_area_faces==1 &&
            zero_report.topology.repeated_index_faces==0,
            "distinct-index collinear face reported geometrically degenerate");

    const auto reversed=parse(
        "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\n"
        "f 1 2 3\n"
        "f 1 4 3\n" // shared 1-3 now same direction on both faces
    );
    const auto mis=book03::inspect_mesh(reversed.topological_mesh());
    require(mis.winding_conflict_edges==1,
            "face grammar success does not prove winding consistency");

    const auto meta=parse(
        "o Teapot\n"
        "g side\n"
        "s 1\n"
        "usemtl paint\n"
        "mtllib paint.mtl\n"
        "v 0 0 0\nv 1 0 0\nv 0 1 0\n"
        "vn 0 0 0\n"
        "f 1//1 2//1 3//1\n"
    );
    const auto m=book03::inspect_obj_asset(meta);
    require(m.warnings==5 && m.invalid_supplied_normals==1,
            "unimplemented metadata and zero normals are explicit warnings");
}

void malformed_and_adversarial_obj_inputs_are_rejected() {
    const std::string start=
        "v 0 0 0\nv 1 0 0\nv 0 1 0\n";
    const std::vector<std::string> bad{
        "f 0 2 3\n",
        "f 4 2 3\n",
        "f -4 -2 -1\n",
        "f 1/1 2 3\n",
        "f 1// 2 3\n",
        "f 1/ 2 3\n",
        "f 1/2/3/4 2 3\n",
        "f 1 2\n",
        "v 7 7 7 7\nf 1 2 3\n",
        "vt 1 2 3\nf 1 2 3\n",
        "f 1 2 3 4 5\n",
        "curv 1 2 3\nf 1 2 3\n",
        "f 1 2 +99999999999999999999999999\n"
    };
    for(const auto& suffix:bad)
        rejects<book03::ObjInputError>(
            [&]{(void)parse(start+suffix);},
            "malformed OBJ grammar or bounds must reject explicitly");
    rejects<book03::ObjInputError>(
        [&]{(void)parse("v nan 0 0\n"+start+"f 1 2 3\n");},
        "nonfinite vertex rejected");
    rejects<book03::ObjInputError>(
        [&]{(void)parse(start+"f 1 2 3\n",
            {.max_lines=3,.max_line_chars=4096,.max_positions=10,
             .max_uvs=10,.max_normals=10,.max_faces=10});},
        "maximum line count enforced");
    rejects<book03::ObjInputError>(
        [&]{(void)parse(start+"f 1 2 3\n",
            {.max_lines=20,.max_line_chars=6,.max_positions=10,
             .max_uvs=10,.max_normals=10,.max_faces=10});},
        "maximum line length enforced");
    rejects<std::invalid_argument>(
        [&]{(void)parse(start+"f 1 2 3\n",
            {.max_lines=0});},
        "zero-sized safety limits rejected");
    rejects<book03::ObjInputError>(
        [&]{(void)parse(start+"f 1 2 3\nf 1 3 2\n",
            {.max_faces=1});},
        "face count budget enforced without wraparound");
}

void gltf_bounds_stride_alignment_and_overflow_tests() {
    using book03::GltfAccessor;
    using book03::GltfBufferView;
    using book03::GltfComponent;
    using book03::GltfType;
    using book03::check_gltf_layout;
    const auto layout=check_gltf_layout(256,
        GltfBufferView{.byte_offset=16,.byte_length=128,
                       .byte_stride=24},
        GltfAccessor{.component=GltfComponent::Float,
                     .type=GltfType::Vec3,.byte_offset=8,.count=5});
    require(layout.element_size==12 &&
            layout.effective_stride==24 &&
            layout.first_byte==24 &&
            layout.after_last_byte==132,
            "glTF buffer + view + accessor two offsets and stride accounted");
    const auto compact=check_gltf_layout(24,
        GltfBufferView{0,24,0},
        GltfAccessor{GltfComponent::UnsignedShort,GltfType::Vec3,0,4});
    require(compact.element_size==6 &&
            compact.effective_stride==6 &&
            compact.after_last_byte==24,
            "tight packed vec3 ushort count and layout");
    const auto scalar=check_gltf_layout(8,
        GltfBufferView{0,8,0},
        GltfAccessor{GltfComponent::UnsignedInt,GltfType::Scalar,0,2});
    require(scalar.element_size==4&&scalar.after_last_byte==8,
            "index scalar UINT32 fits exact byte extent");
    rejects<std::out_of_range>(
        [&]{(void)check_gltf_layout(32,
            GltfBufferView{16,32,0},
            GltfAccessor{GltfComponent::Float,GltfType::Vec3,0,1});},
        "view outside physical buffer rejected with otherwise valid accessor");
    // An independent invalid-count witness must not mask the bounds witness.
    rejects<std::invalid_argument>(
        [&]{(void)check_gltf_layout(64,
            GltfBufferView{0,64,0},GltfAccessor{});},
        "zero accessor count rejected even in a valid buffer view");
    rejects<std::out_of_range>(
        [&]{(void)check_gltf_layout(16,
            GltfBufferView{0,16,0},
            GltfAccessor{GltfComponent::Float,GltfType::Vec3,0,2});},
        "accessor reading past view rejected");
    rejects<std::invalid_argument>(
        [&]{(void)check_gltf_layout(64,
            GltfBufferView{0,64,8},
            GltfAccessor{GltfComponent::Float,GltfType::Vec3,0,2});},
        "stride smaller than element size rejected");
    rejects<std::invalid_argument>(
        [&]{(void)check_gltf_layout(64,
            GltfBufferView{0,64,16},
            GltfAccessor{GltfComponent::UnsignedShort,GltfType::Vec3,1,2});},
        "misaligned accessor byteOffset rejected");
    rejects<std::invalid_argument>(
        [&]{(void)check_gltf_layout(64,
            GltfBufferView{0,64,0},
            GltfAccessor{GltfComponent::Float,GltfType::Vec3,0,1,true});},
        "normalized FLOAT accessor is not a valid interpretation");
    rejects<std::invalid_argument>(
        [&]{(void)check_gltf_layout(64,
            GltfBufferView{0,64,0},
            GltfAccessor{GltfComponent::Float,GltfType::Vec3,0,1,false,true});},
        "sparse accessor needs a separate decoder and is refused");
    rejects<std::out_of_range>(
        [&]{(void)check_gltf_layout(
            std::numeric_limits<std::size_t>::max(),
            GltfBufferView{0,64,0},
            GltfAccessor{GltfComponent::UnsignedInt,GltfType::Vec4,0,
                         std::numeric_limits<std::size_t>::max()});},
        "huge accessor count cannot overflow bounds calculation");
}

} // namespace

int main() {
    try {
        separate_index_domains_and_topological_identity();
        independent_negative_indices_and_face_time_scopes();
        convex_quad_triangulation_is_explicitly_restricted();
        parsing_does_not_imply_geometric_or_topological_validity();
        malformed_and_adversarial_obj_inputs_are_rejected();
        gltf_bounds_stride_alignment_and_overflow_tests();
        std::cout<<"Book03/P10: OBJ grammar, tuple seams, topology audit "
                    "and glTF bounds checks: PASS\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"Book03/P10 FAILED: "<<error.what()<<'\n';
        return 1;
    }
}
