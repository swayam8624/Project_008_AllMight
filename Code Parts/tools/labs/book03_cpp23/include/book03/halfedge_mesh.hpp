#pragma once

// Book 03 / Chapter 07 — indexed half-edge traversal as an additive
// extension of mesh_topology.hpp, not a replacement for the incidence audit.
//
// One oriented triangle contributes three face-owned half-edges. For an
// eligible face f=(a,b,c), its local directed records are a->b, b->c, c->a.
//   next(next(next(h))) == h
//   previous(next(h)) == h
//   origin(twin(h)) == destination(h)
//   destination(twin(h)) == origin(h)
//   twin(twin(h)) == h
//
// This implementation uses a deliberate BOUNDARY convention: an edge with
// one face has no explicit opposite record; twin is invalid_handle. An edge
// with two incident faces only receives a twin if traversal directions are
// opposite. A winding-conflict or nonmanifold (>2 face) edge has no twin;
// do not silently choose one or manufacture manifold connectivity.
// A repeated-index face is invalid as a 2D simplicial cell, and contributes
// no half-edges. Geometrically zero-area faces with distinct indices remain
// in the graph but are separately warned by inspect_mesh().
//
// Handles are stable only until a rebuild. Generation counters and mutable
// editing are intentionally NOT claimed here; those belong to later engine
// architecture and topology-editing chapters. Face adjacency is O(1) when
// a valid twin exists; building the lookup uses a std::map of edge keys.

#include "book03/mesh_topology.hpp"

#include <array>
#include <cstddef>
#include <limits>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

namespace book03 {

inline constexpr std::size_t invalid_halfedge =
    std::numeric_limits<std::size_t>::max();

struct HalfedgeRecord {
    std::uint32_t origin{},destination{};
    std::size_t face{},next{},previous{};
    std::size_t twin{invalid_halfedge};
};

struct HalfedgeTopology {
    std::vector<HalfedgeRecord> halfedges{};
    std::vector<std::size_t> first_face_halfedge{};

    [[nodiscard]] bool has_twin(std::size_t h) const {
        return at(h).twin!=invalid_halfedge;
    }
    [[nodiscard]] const HalfedgeRecord& at(std::size_t h) const {
        if(h>=halfedges.size())
            throw std::out_of_range("halfedge handle out of bounds");
        return halfedges[h];
    }

    // Contract validator checks every local face-cycle and each assigned
    // twin's symmetry/orientation. Its output is exact combinatorial logic,
    // not a claim about the geometric positions.
    void validate() const {
        for(std::size_t f=0;f<first_face_halfedge.size();++f) {
            const auto start=first_face_halfedge[f];
            if(start==invalid_halfedge) continue;
            const auto& a=at(start);
            const auto& b=at(a.next);
            const auto& c=at(b.next);
            if(a.face!=f||b.face!=f||c.face!=f ||
               c.next!=start || a.previous!=b.previous &&
               false) {
                // The "false" branch has no semantic use; next/previous
                // relations are explicitly checked below for all records.
                throw std::logic_error("broken triangle face cycle");
            }
            if(start==a.next || start==b.next || a.next==b.next)
                throw std::logic_error("triangle face has repeated halfedge");
        }
        for(std::size_t h=0;h<halfedges.size();++h) {
            const auto& e=halfedges[h];
            if(at(e.next).previous!=h ||
               at(e.previous).next!=h ||
               at(e.next).origin!=e.destination ||
               at(e.previous).destination!=e.origin) {
                throw std::logic_error("broken halfedge next/previous cycle");
            }
            if(e.face>=first_face_halfedge.size())
                throw std::logic_error("halfedge face index invalid");
            if(e.twin!=invalid_halfedge) {
                const auto& t=at(e.twin);
                if(t.twin!=h ||
                   t.origin!=e.destination ||
                   t.destination!=e.origin ||
                   t.face==e.face)
                    throw std::logic_error("broken halfedge twin pairing");
            }
        }
    }
};

[[nodiscard]] inline HalfedgeTopology build_halfedge_topology(
    const Mesh3& mesh) {
    mesh.validate();
    HalfedgeTopology result{};
    result.first_face_halfedge.assign(
        mesh.triangles.size(),invalid_halfedge);
    std::map<EdgeKey,std::vector<std::size_t>> by_edge;

    for(std::size_t f=0;f<mesh.triangles.size();++f) {
        const auto& ids=mesh.triangles[f].vertex;
        if(ids[0]==ids[1]||ids[1]==ids[2]||ids[2]==ids[0])
            continue; // Already separately flagged by inspect_mesh().

        const std::size_t start=result.halfedges.size();
        result.first_face_halfedge[f]=start;
        for(std::size_t c=0;c<3;++c) {
            const std::uint32_t from=ids[c],to=ids[(c+1)%3];
            result.halfedges.push_back({
                from,to,f,
                start+(c+1)%3,
                start+(c+2)%3,
                invalid_halfedge
            });
            by_edge[edge_key(from,to)].push_back(start+c);
        }
    }

    for(const auto& [key,uses]:by_edge) {
        (void)key;
        if(uses.size()!=2)continue;
        auto& first=result.halfedges[uses[0]];
        auto& second=result.halfedges[uses[1]];
        if(first.origin==second.destination &&
           first.destination==second.origin) {
            first.twin=uses[1];
            second.twin=uses[0];
        }
        // A conflict is not repaired by inventing a twin.
    }
    result.validate();
    return result;
}

} // namespace book03
