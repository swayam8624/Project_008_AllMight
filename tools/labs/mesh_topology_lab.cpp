// PROJECTALLMIGHT Phase 27 witness: half-edge pairing, manifold/boundary checks, and Euler characteristic.
#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

struct Face{ std::array<int,3> v{}; };
struct HalfEdge{ int from=-1,to=-1,face=-1,twin=-1; };

struct Topology{
    int vertex_count=0;
    std::vector<Face> faces;
    std::vector<HalfEdge> halfedges;
    int unique_edges=0;
    int boundary_edges=0;
    int nonmanifold_edges=0;
    int orientation_conflicts=0;
};

Topology build_topology(int vertex_count,const std::vector<Face>& faces){
    require(vertex_count>=0,"negative vertex count");
    Topology out;
    out.vertex_count=vertex_count;
    out.faces=faces;

    std::map<std::pair<int,int>,std::vector<int>> undirected;

    for(std::size_t fi=0;fi<faces.size();++fi){
        const auto f=faces[fi].v;
        require(f[0]!=f[1]&&f[1]!=f[2]&&f[2]!=f[0],"triangle repeats a vertex");
        for(int v:f) require(v>=0&&v<vertex_count,"face vertex out of range");

        for(int e=0;e<3;++e){
            const int a=f[e], b=f[(e+1)%3];
            const int hi=static_cast<int>(out.halfedges.size());
            out.halfedges.push_back({a,b,static_cast<int>(fi),-1});
            const auto key=std::minmax(a,b);
            undirected[{key.first,key.second}].push_back(hi);
            // Do not reject repeated directed incidence here. Three or more
            // faces can share an undirected edge, in which case duplicate
            // directions are unavoidable. Collect the complete incidence first
            // and classify manifoldness/orientation below.
        }
    }

    out.unique_edges=static_cast<int>(undirected.size());
    for(const auto& [edge,list]:undirected){
        (void)edge;
        if(list.size()==1){
            ++out.boundary_edges;
        }else if(list.size()==2){
            HalfEdge& a=out.halfedges[static_cast<std::size_t>(list[0])];
            HalfEdge& b=out.halfedges[static_cast<std::size_t>(list[1])];
            if(a.from==b.to && a.to==b.from){
                a.twin=list[1];
                b.twin=list[0];
            }else{
                ++out.orientation_conflicts;
            }
        }else{
            ++out.nonmanifold_edges;
        }
    }
    return out;
}

int euler_characteristic(const Topology& t){
    return t.vertex_count-t.unique_edges+static_cast<int>(t.faces.size());
}

void test_disk_mesh(){
    const std::vector<Face> f{{{{0,1,2}}},{{{0,2,3}}}};
    const Topology t=build_topology(4,f);
    require(t.unique_edges==5,"two-triangle disk should have five unique edges");
    require(t.boundary_edges==4,"square disk should have four boundary edges");
    require(t.nonmanifold_edges==0&&t.orientation_conflicts==0,"disk should be oriented manifold with boundary");
    require(euler_characteristic(t)==1,"disk Euler characteristic should be one");

    int twins=0;
    for(const HalfEdge& h:t.halfedges) if(h.twin>=0) ++twins;
    require(twins==2,"one interior undirected edge should create two twin half-edges");
}

void test_closed_tetrahedron(){
    const std::vector<Face> f{
        {{{0,2,1}}},
        {{{0,1,3}}},
        {{{1,2,3}}},
        {{{2,0,3}}}
    };
    const Topology t=build_topology(4,f);
    require(t.unique_edges==6,"tetrahedron should have six edges");
    require(t.boundary_edges==0,"closed tetrahedron has no boundary");
    require(t.nonmanifold_edges==0&&t.orientation_conflicts==0,"tetrahedron should be oriented two-manifold");
    require(euler_characteristic(t)==2,"sphere-like tetrahedron Euler characteristic should be two");
}

void test_nonmanifold_edge(){
    const std::vector<Face> f{
        {{{0,1,2}}},
        {{{1,0,3}}},
        {{{0,1,4}}}
    };
    const Topology t=build_topology(5,f);
    require(t.nonmanifold_edges==1,"three triangles sharing one edge should be nonmanifold");
}

} // namespace

int main(){
    try{
        test_disk_mesh();
        test_closed_tetrahedron();
        test_nonmanifold_edge();
        std::cout<<"mesh_topology_lab: PASS\n"
                 <<"proved: unique-edge construction, half-edge twin pairing, boundary/nonmanifold classification, orientation check, Euler characteristic on disk and closed tetrahedron\n"
                 <<"not proved: vertex-manifold fans, arbitrary polygon meshes, robust geometric predicates, edge collapse, QEM, remeshing, subdivision, or production importer parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"mesh_topology_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
