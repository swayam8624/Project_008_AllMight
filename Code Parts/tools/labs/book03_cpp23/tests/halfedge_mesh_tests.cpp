// Book 03 / P07 — half-edge face-cycle, twin and unsafe-incidence tests.
// Source-only test declarations are not evidence of an executed binary.

#include "book03/halfedge_mesh.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>

namespace {
using book03::Mesh3;
using book03::HalfedgeTopology;
constexpr std::size_t absent=book03::invalid_halfedge;

void require(bool good,std::string_view message) {
    if(!good) throw std::runtime_error(std::string(message));
}

std::size_t with_twin(const HalfedgeTopology& mesh) {
    return static_cast<std::size_t>(std::count_if(
        mesh.halfedges.begin(),mesh.halfedges.end(),
        [](const auto& h){return h.twin!=absent;}));
}

void face_cycles_and_quad_twin() {
    const auto m=book03::reference_quad();
    const auto h=book03::build_halfedge_topology(m);
    h.validate();
    require(h.first_face_halfedge.size()==2,"quad has two faces");
    require(h.halfedges.size()==6,"each triangle contributes three halfedges");
    require(with_twin(h)==2,"one indexed diagonal makes one twin pair");
    const auto first=h.first_face_halfedge[0];
    require(first!=absent,"eligible face has first halfedge");
    const auto& e=h.at(first);
    require(e.origin==0 && e.destination==1 && e.face==0,
            "explicit directed first edge");
    require(h.at(h.at(h.at(first).next).next).next==first,
            "next three times returns to face start");
    require(h.at(h.at(first).next).previous==first,
            "previous inverse of next");

    bool matched=false;
    for(std::size_t i=0;i<h.halfedges.size();++i) {
        const auto& use=h.halfedges[i];
        if(use.origin==2 && use.destination==0) {
            require(use.twin!=absent,"diagonal has twin");
            const auto& t=h.at(use.twin);
            require(t.origin==0&&t.destination==2&&t.twin==i,
                    "twin exchanges endpoints and is symmetric");
            matched=true;
        }
    }
    require(matched,"the known diagonal must be present");
}

void closed_house_every_halfedge_has_a_twin() {
    const auto mesh=book03::reference_house();
    const auto report=book03::inspect_mesh(mesh);
    require(report.closed_orientable_edge_candidate(),
            "house passes edge and fan checks");
    const auto half=book03::build_halfedge_topology(mesh);
    half.validate();
    require(half.halfedges.size()==48,
            "16 triangle faces have 48 directed halfedges");
    require(with_twin(half)==48,
            "24 consistently shared edges make 24 twin pairs");
    for(std::size_t f=0;f<mesh.triangles.size();++f)
        require(half.first_face_halfedge[f]!=absent,
                "every house face has a valid face cycle");
}

void boundary_conflict_and_nonmanifold_do_not_invent_twins() {
    const auto seam=book03::build_halfedge_topology(
        book03::fixture_mesh("seam"));
    seam.validate();
    require(seam.halfedges.size()==6 && with_twin(seam)==0,
            "coincident but disconnected faces have no twins");

    const auto flipped=book03::build_halfedge_topology(
        book03::fixture_mesh("flipped"));
    flipped.validate();
    require(flipped.halfedges.size()==6 && with_twin(flipped)==0,
            "same-directed edge uses are not legal twins");

    const auto nonmanifold=book03::build_halfedge_topology(
        book03::fixture_mesh("nonmanifold"));
    nonmanifold.validate();
    require(nonmanifold.halfedges.size()==9,
            "three faces yield nine halfedges");
    require(with_twin(nonmanifold)==0,
            "three-way edge must not receive arbitrary twin");

    const auto bowtie=book03::build_halfedge_topology(
        book03::fixture_mesh("bowtie"));
    bowtie.validate();
    require(bowtie.halfedges.size()==6&&with_twin(bowtie)==0,
            "vertex-only adjacency has no shared halfedge twin");
}

void degenerate_face_is_excluded_without_losing_face_index() {
    const auto mesh=book03::fixture_mesh("degenerate");
    const auto h=book03::build_halfedge_topology(mesh);
    h.validate();
    require(h.first_face_halfedge.size()==3,"input face ID space preserved");
    require(h.first_face_halfedge[2]==absent,
            "repeated-index face has no valid 3-cycle");
    require(h.halfedges.size()==6,
            "two eligible triangles still yield six halfedges");
}

void validator_detects_corrupted_halfedge_links() {
    auto h=book03::build_halfedge_topology(book03::reference_quad());
    h.halfedges[0].next=0;
    bool failed=false;
    try {h.validate();}
    catch(const std::logic_error&) {failed=true;}
    require(failed,"validator rejects a corrupted next edge");

    auto h2=book03::build_halfedge_topology(book03::reference_quad());
    for(auto& e:h2.halfedges)if(e.twin!=absent) {
        e.twin=absent;
        break;
    }
    failed=false;
    try {h2.validate();}
    catch(const std::logic_error&) {failed=true;}
    require(failed,"validator rejects one-sided twin pointer");
}

} // namespace

int main() {
    try {
        face_cycles_and_quad_twin();
        closed_house_every_halfedge_has_a_twin();
        boundary_conflict_and_nonmanifold_do_not_invent_twins();
        degenerate_face_is_excluded_without_losing_face_index();
        validator_detects_corrupted_halfedge_links();
        std::cout<<"Book03/P07: halfedge face-cycle and twin contracts: PASS\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"Book03/P07 halfedge FAILED: "<<error.what()<<'\n';
        return 1;
    }
}
