#pragma once
// Book03/Chapter10: a deliberately bounded, inspectable OBJ triangle/convex
// quad importer and glTF buffer-layout validator; standard C++23 only.
//
// IMPORT IS NOT TRUST. The OBJ parser checks syntax, index scope, finite
// numeric values, stream size and polygon eligibility. The P07 topology
// validator separately reports holes, repeated-index/zero-area faces,
// winding conflicts, disconnected vertex fans and nonmanifold edges.
//
// OBJ corner indices are (position, UV, normal) independent streams. Our
// Mesh3 uses POSITION indices only, preserving true mesh adjacency, while
// rendering streams unify full CORNER TUPLES. Never weld by XYZ tolerance.
// We don't invent UVs or normals when the file omitted them.
//
// Supported OBJ: ASCII v x y z, vt u v, vn x y z, f p[/t[/n]], triangles,
// and planar strictly-convex quads. Positive indices (one-based) and
// negative relative indices are resolved against stream lengths AT THE
// LINE OF THE FACE. No forward positive references, zero index, self-
// intersecting/concave/warped polygons, homogeneous v/w, more-than-two
// texture components, per-face colors, OBJ freeform curves, external MTL,
// smoothing groups, N-gons beyond a quad, units or axis transforms.
// Metadata o/g/s/usemtl/mtllib lines are recognized but ignored with a
// recorded warning; an unknown directive is rejected, never hidden.
// Parsing a face is NOT proof that its geometry has nonzero area.
//
// glTF teaching subset: bounds/offset/stride/component/count validator
// for an already decoded buffer view/accessor. It is NOT JSON, GLB,
// animation, material or sparse-accessor ingestion. No byte reads happen.
// Real glTF has additional alignment, extension, type and scene rules.

#include "book03/mesh_topology.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <limits>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace book03 {

struct ObjInputError final:std::runtime_error {
    std::size_t line{};
    ObjInputError(std::size_t n,const std::string& detail)
        :std::runtime_error("OBJ line "+std::to_string(n)+": "+detail),line(n){}
};
struct ObjImportLimits {
    std::size_t max_lines{50000};
    std::size_t max_line_chars{4096};
    std::size_t max_positions{25000};
    std::size_t max_uvs{25000};
    std::size_t max_normals{25000};
    std::size_t max_faces{50000};
};
// OBJ face corner's three indices: -1 means attribute not provided, NEVER
// a resolved negative input index (those resolve to zero-based >=0).
struct ObjCornerKey {
    std::uint32_t position{};
    std::optional<std::uint32_t> texcoord{};
    std::optional<std::uint32_t> normal{};
    [[nodiscard]] friend auto operator<=>(const ObjCornerKey&,
                                            const ObjCornerKey&)=default;
};
struct ObjTriangle {
    std::array<ObjCornerKey,3> corners{};
};
struct ObjRenderingVertex {
    ObjCornerKey key{};
    Point3 position{};
    std::optional<UV2> uv{};
    std::optional<Vector3> normal{};
};
struct ObjAsset {
    std::vector<Point3> positions{};
    std::vector<UV2> texcoords{};
    std::vector<Vector3> normals{};
    std::vector<ObjTriangle> faces{};
    std::vector<std::string> warnings{};

    // Logical geometric mesh: shared POSITION identity. Rendering UV/normal
    // seams are not allowed to tear this connectivity accidentally.
    [[nodiscard]] Mesh3 topological_mesh() const {
        Mesh3 m{};
        m.positions=positions;
        for(const auto& face:faces)
            m.triangles.push_back({{
                face.corners[0].position,
                face.corners[1].position,
                face.corners[2].position
            }});
        m.validate();
        return m;
    }

    // Rendering streams use full tuples, not positions alone. The stable
    // first-seen assignment is deterministic for a given input face order.
    struct RenderingMesh {
        std::vector<ObjRenderingVertex> vertices{};
        std::vector<TriangleIndices> triangles{};
    };
    [[nodiscard]] RenderingMesh to_rendering_mesh() const {
        RenderingMesh result{};
        std::map<ObjCornerKey,std::uint32_t> by_tuple{};
        for(const auto& face:faces) {
            TriangleIndices indices{};
            for(int c=0;c<3;++c) {
                const auto& key=face.corners[c];
                auto [it,added]=by_tuple.try_emplace(
                    key,static_cast<std::uint32_t>(result.vertices.size()));
                if(added) {
                    if(key.position>=positions.size())
                        throw std::logic_error("bad position reference");
                    ObjRenderingVertex v{};
                    v.key=key;
                    v.position=positions[key.position];
                    if(key.texcoord) {
                        if(*key.texcoord>=texcoords.size())
                            throw std::logic_error("bad UV reference");
                        v.uv=texcoords[*key.texcoord];
                    }
                    if(key.normal) {
                        if(*key.normal>=normals.size())
                            throw std::logic_error("bad normal reference");
                        v.normal=normals[*key.normal];
                    }
                    result.vertices.push_back(v);
                }
                indices.vertex[c]=it->second;
            }
            result.triangles.push_back(indices);
        }
        return result;
    }
};

[[nodiscard]] inline double obj_decimal(
    std::string_view text,std::size_t line) {
    if(text.empty())throw ObjInputError(line,"empty decimal number");
    double result=0;
    const auto [end,error]=std::from_chars(
        text.data(),text.data()+text.size(),result,
        std::chars_format::general);
    if(error!=std::errc{}||end!=text.data()+text.size()||
       !std::isfinite(result))
        throw ObjInputError(line,"malformed or nonfinite decimal");
    return result;
}
[[nodiscard]] inline std::uint32_t obj_resolve_index(
    std::string_view number,std::size_t count,std::size_t line) {
    if(number.empty())
        throw ObjInputError(line,"missing position/UV/normal index");
    std::int64_t id=0;
    const auto [end,error]=std::from_chars(
        number.data(),number.data()+number.size(),id);
    if(error!=std::errc{}||end!=number.data()+number.size()||id==0)
        throw ObjInputError(line,"invalid OBJ nonzero integer index");
    // Positive indices are 1-based; negatives refer to current stream
    // length. Both are validated BEFORE converting to uint32.
    const std::int64_t n=static_cast<std::int64_t>(count);
    const std::int64_t resolved=id>0?id-1:n+id;
    if(resolved<0||resolved>=n ||
       resolved>std::numeric_limits<std::uint32_t>::max())
        throw ObjInputError(line,"OBJ reference out of current stream bounds");
    return static_cast<std::uint32_t>(resolved);
}
[[nodiscard]] inline ObjCornerKey obj_corner(
    std::string_view word,std::size_t pcount,std::size_t tcount,
    std::size_t ncount,std::size_t line) {
    const auto slash=word.find('/');
    const auto first=word.substr(0,slash);
    ObjCornerKey result{};
    result.position=obj_resolve_index(first,pcount,line);
    if(slash==std::string_view::npos)return result;
    const auto slash2=word.find('/',slash+1);
    const auto second=word.substr(slash+1,
        slash2==std::string_view::npos?
        std::string_view::npos:slash2-slash-1);
    if(!second.empty())
        result.texcoord=obj_resolve_index(second,tcount,line);
    if(slash2!=std::string_view::npos) {
        if(word.find('/',slash2+1)!=std::string_view::npos)
            throw ObjInputError(line,"too many OBJ index streams");
        const auto third=word.substr(slash2+1);
        if(third.empty())
            throw ObjInputError(line,"trailing missing normal index");
        result.normal=obj_resolve_index(third,ncount,line);
    } else if(second.empty()) {
        throw ObjInputError(line,"empty UV without //normal is invalid");
    }
    return result;
}
[[nodiscard]] inline std::vector<std::string> obj_tokens(
    const std::string& line) {
    const auto hash=line.find('#');
    std::istringstream input{line.substr(0,hash)};
    std::vector<std::string> result{};
    std::string item{};
    while(input>>item)result.push_back(std::move(item));
    return result;
}

// A single convex quad can be split without a general polygon triangulator.
// We check no repeated indices, 3D planarity relative to scale, and strict
// convexity after dominant-axis projection. This is not a robust exact
// predicate; near-degenerate quads are rejected by a documented tolerance.
inline void obj_check_convex_quad(const ObjAsset& a,
                                  const std::array<ObjCornerKey,4>& q,
                                  std::size_t line) {
    for(int i=0;i<4;++i)
        for(int j=i+1;j<4;++j)
            if(q[i].position==q[j].position)
                throw ObjInputError(line,"repeated position index in quad");
    const Point3 p0=a.positions[q[0].position],
                 p1=a.positions[q[1].position],
                 p2=a.positions[q[2].position],
                 p3=a.positions[q[3].position];
    const Vector3 n=cross(p1-p0,p2-p0);
    const double area2=magnitude(n);
    const double scale=std::max({magnitude(p1-p0),
        magnitude(p2-p0),magnitude(p3-p0)});
    if(!(scale>0)||!(area2>1e-12*scale*scale))
        throw ObjInputError(line,"quad first triangle nearly degenerate");
    if(std::abs(dot(n,p3-p0))>1e-10*area2*scale)
        throw ObjInputError(line,"quad nonplanar beyond tolerance");

    const auto ax=std::abs(n.x),ay=std::abs(n.y),az=std::abs(n.z);
    const int drop=ax>=ay&&ax>=az?0:ay>=az?1:2;
    const auto flatten=[drop](Point3 p)->std::array<double,2> {
        return drop==0?std::array{p.y,p.z}:
               drop==1?std::array{p.x,p.z}:
                       std::array{p.x,p.y};
    };
    std::array<std::array<double,2>,4> xy{
        flatten(p0),flatten(p1),flatten(p2),flatten(p3)
    };
    double sign=0.0;
    for(int i=0;i<4;++i) {
        const auto& A=xy[i];
        const auto& B=xy[(i+1)%4];
        const auto& C=xy[(i+2)%4];
        const double x=(B[0]-A[0])*(C[1]-B[1])-
                       (B[1]-A[1])*(C[0]-B[0]);
        if(!std::isfinite(x)||std::abs(x)<=1e-12*scale*scale)
            throw ObjInputError(line,"quad non-strict convexity/collinearity");
        if(i==0)sign=x;
        else if((x>0)!=(sign>0))
            throw ObjInputError(line,"concave or self-intersecting quad");
    }
}

// Do NOT call std::getline on an unlimited untrusted file line and only
// check the length after its allocation. Read with an explicit byte budget.
// CRLF is accepted; the trailing CR is removed on a completed line.
[[nodiscard]] inline bool obj_bounded_line(
    std::istream& input,std::string& line,
    std::size_t max_chars,std::size_t line_number) {
    line.clear();
    for(;;) {
        const int ch=input.get();
        if(ch==std::char_traits<char>::eof()) {
            if(input.bad())throw std::runtime_error("OBJ stream I/O failure");
            if(!line.empty()&&line.back()=='\r')line.pop_back();
            return !line.empty();
        }
        if(ch=='\n') {
            if(!line.empty()&&line.back()=='\r')line.pop_back();
            return true;
        }
        if(line.size()>=max_chars)
            throw ObjInputError(line_number,"input line too long");
        line.push_back(static_cast<char>(ch));
    }
}

[[nodiscard]] inline ObjAsset import_obj(
    std::istream& source,ObjImportLimits limits={}) {
    if(limits.max_lines==0||limits.max_line_chars==0||
       limits.max_positions==0||limits.max_uvs==0||
       limits.max_normals==0||limits.max_faces==0)
        throw std::invalid_argument("OBJ limits must be positive");
    ObjAsset a{};
    std::size_t line_num=0;
    std::string line{};
    while(obj_bounded_line(source,line,limits.max_line_chars,
                           line_num+1)) {
        ++line_num;
        if(line_num>limits.max_lines)
            throw ObjInputError(line_num,"input line count exceeds limit");
        const auto words=obj_tokens(line);
        if(words.empty())continue;
        const auto& kind=words[0];
        auto required=[&](std::size_t n) {
            if(words.size()!=n)
                throw ObjInputError(line_num,
                    "unexpected number of fields in "+kind);
        };
        if(kind=="v") {
            required(4);
            if(a.positions.size()>=limits.max_positions)
                throw ObjInputError(line_num,"too many OBJ positions");
            a.positions.push_back({
                obj_decimal(words[1],line_num),
                obj_decimal(words[2],line_num),
                obj_decimal(words[3],line_num)
            });
        } else if(kind=="vt") {
            required(3);
            if(a.texcoords.size()>=limits.max_uvs)
                throw ObjInputError(line_num,"too many OBJ UV values");
            a.texcoords.push_back({
                obj_decimal(words[1],line_num),
                obj_decimal(words[2],line_num)
            });
        } else if(kind=="vn") {
            required(4);
            if(a.normals.size()>=limits.max_normals)
                throw ObjInputError(line_num,"too many OBJ normal records");
            a.normals.push_back({
                obj_decimal(words[1],line_num),
                obj_decimal(words[2],line_num),
                obj_decimal(words[3],line_num)
            });
        } else if(kind=="f") {
            if(words.size()<4||words.size()>5)
                throw ObjInputError(line_num,
                    "supported OBJ faces must be triangles or quads");
            const std::size_t additional=words.size()==5?2:1;
            if(additional>limits.max_faces ||
               a.faces.size()>limits.max_faces-additional)
                throw ObjInputError(line_num,"too many OBJ triangles");
            std::array<ObjCornerKey,4> q{};
            for(std::size_t i=1;i<words.size();++i)
                q[i-1]=obj_corner(words[i],a.positions.size(),
                       a.texcoords.size(),a.normals.size(),line_num);
            if(words.size()==5)
                obj_check_convex_quad(a,q,line_num);
            a.faces.push_back({{q[0],q[1],q[2]}});
            if(words.size()==5)
                a.faces.push_back({{q[0],q[2],q[3]}});
        } else if(kind=="o"||kind=="g"||kind=="s"||
                  kind=="usemtl"||kind=="mtllib") {
            // A known but unsupported semantic can alter material/group/
            // smoothing interpretation. Record explicitly; do NOT claim
            // it was imported.
            a.warnings.push_back("line "+std::to_string(line_num)+
                ": ignored OBJ metadata directive "+kind);
        } else {
            throw ObjInputError(line_num,
                "unsupported OBJ directive "+kind);
        }
    }
    if(source.bad())throw std::runtime_error("OBJ stream I/O failure");
    if(a.positions.empty()||a.faces.empty())
        throw std::runtime_error("OBJ needs positions and supported faces");
    // Each face may still be degenerate/nonmanifold. The importer does not
    // erase that evidence: ask P07 for a separate mesh integrity report.
    a.topological_mesh().validate();
    return a;
}

struct AssetReport {
    MeshReport topology{};
    std::size_t geometric_vertices{};
    std::size_t rendering_vertices{};
    std::size_t triangles{};
    std::size_t warnings{};
    std::size_t invalid_supplied_normals{};
    Point3 bounds_min{},bounds_max{};
};
[[nodiscard]] inline AssetReport inspect_obj_asset(const ObjAsset& a) {
    AssetReport result{};
    result.topology=inspect_mesh(a.topological_mesh());
    result.geometric_vertices=a.positions.size();
    result.rendering_vertices=a.to_rendering_mesh().vertices.size();
    result.triangles=a.faces.size();
    result.warnings=a.warnings.size();
    for(const auto& n:a.normals)
        if(!finite(n)||magnitude(n)==0)
            ++result.invalid_supplied_normals;
    result.bounds_min=result.bounds_max=a.positions.front();
    for(Point3 p:a.positions) {
        result.bounds_min.x=std::min(result.bounds_min.x,p.x);
        result.bounds_min.y=std::min(result.bounds_min.y,p.y);
        result.bounds_min.z=std::min(result.bounds_min.z,p.z);
        result.bounds_max.x=std::max(result.bounds_max.x,p.x);
        result.bounds_max.y=std::max(result.bounds_max.y,p.y);
        result.bounds_max.z=std::max(result.bounds_max.z,p.z);
    }
    return result;
}

// glTF 2.0 bufferView/accessor subset. Effective byte address for element i:
//
// bufferView.byteOffset + accessor.byteOffset + i*effectiveStride.
//
// Check all additions/multiplications by division/subtraction, NOT just a
// potentially wrapped end offset. Component/count/type must be reconciled
// with the consuming attribute semantic. This checks byte layout only.
enum class GltfComponent:std::uint32_t {
    UnsignedByte=5121,UnsignedShort=5123,UnsignedInt=5125,Float=5126
};
enum class GltfType:std::uint8_t {
    Scalar=1,Vec2=2,Vec3=3,Vec4=4
};
struct GltfBufferView {
    std::size_t byte_offset{};
    std::size_t byte_length{};
    std::size_t byte_stride{}; // 0 => tightly packed
};
struct GltfAccessor {
    GltfComponent component{GltfComponent::Float};
    GltfType type{GltfType::Vec3};
    std::size_t byte_offset{};
    std::size_t count{};
    bool normalized{};
    bool sparse{}; // unsupported by this teaching subset
};
struct GltfLayout {
    std::size_t effective_stride{};
    std::size_t element_size{};
    std::size_t first_byte{};
    std::size_t after_last_byte{};
};
[[nodiscard]] inline GltfLayout check_gltf_layout(
    std::size_t total_buffer_bytes,
    GltfBufferView view,GltfAccessor accessor) {
    if(accessor.sparse)
        throw std::invalid_argument("sparse accessor unsupported in layout lab");
    std::size_t width=0;
    switch(accessor.component) {
        case GltfComponent::UnsignedByte:width=1;break;
        case GltfComponent::UnsignedShort:width=2;break;
        case GltfComponent::UnsignedInt:width=4;break;
        case GltfComponent::Float:width=4;break;
        default:throw std::invalid_argument("unsupported glTF component");
    }
    const auto components=static_cast<std::size_t>(accessor.type);
    if(components<1||components>4||accessor.count==0)
        throw std::invalid_argument("invalid glTF type/count");
    if(accessor.normalized &&
       (accessor.component==GltfComponent::Float ||
        accessor.component==GltfComponent::UnsignedInt))
        throw std::invalid_argument(
            "glTF FLOAT and UNSIGNED_INT cannot be normalized");
    if(view.byte_offset>total_buffer_bytes ||
       view.byte_length>total_buffer_bytes-view.byte_offset)
        throw std::out_of_range("bufferView extends outside buffer");
    if(accessor.byte_offset>view.byte_length)
        throw std::out_of_range("accessor offset outside bufferView");
    const auto elem=components*width;
    const auto stride=view.byte_stride?view.byte_stride:elem;
    if(stride<elem||(stride%width)!=0 ||
       (view.byte_stride && (stride<4||stride>252||stride%4!=0)))
        throw std::invalid_argument("invalid glTF byteStride");
    if((view.byte_offset%width)!=0 ||
       (accessor.byte_offset%width)!=0)
        throw std::invalid_argument("misaligned glTF accessor");
    const auto remaining=view.byte_length-accessor.byte_offset;
    if(elem>remaining ||
       accessor.count-1>(remaining-elem)/stride)
        throw std::out_of_range("accessor element sequence out of bounds");
    // Previous inequalities prove these products/sums cannot wrap as
    // all offsets remain in the already bounded view.
    const auto first=view.byte_offset+accessor.byte_offset;
    const auto after=first+(accessor.count-1)*stride+elem;
    return {stride,elem,first,after};
}

} // namespace book03
