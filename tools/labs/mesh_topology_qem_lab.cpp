// Mesh-processing reference lab: indexed-triangle topology, boundary and
// non-manifold classification, Euler characteristic, plane quadrics and QEM
// edge-collapse cost. This is a CPU oracle, not a production remesher.

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace allmight {

struct V3 { double x{}, y{}, z{}; };
V3 operator-(V3 a,V3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
V3 operator+(V3 a,V3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
V3 operator*(V3 a,double s){return {a.x*s,a.y*s,a.z*s};}
double dot(V3 a,V3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
V3 cross(V3 a,V3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
double length(V3 v){return std::sqrt(dot(v,v));}
V3 normalize(V3 v){double l=length(v); if(!(l>1.0e-12)) throw std::runtime_error("degenerate normal"); return v*(1.0/l);}

using Tri=std::array<int,3>;
using Edge=std::pair<int,int>;

Edge undirected(int a,int b){ return a<b?Edge{a,b}:Edge{b,a}; }

struct Topology {
    std::map<Edge,std::vector<int>> incident_faces;
    std::set<int> used_vertices;

    static Topology build(const std::vector<Tri>& tris){
        Topology t;
        for(std::size_t fi=0;fi<tris.size();++fi){
            const Tri& f=tris[fi];
            if(f[0]==f[1]||f[1]==f[2]||f[2]==f[0]) throw std::runtime_error("degenerate index triangle");
            for(int v:f){
                if(v<0) throw std::runtime_error("negative vertex index");
                t.used_vertices.insert(v);
            }
            for(Edge e:{undirected(f[0],f[1]),undirected(f[1],f[2]),undirected(f[2],f[0])}){
                t.incident_faces[e].push_back(static_cast<int>(fi));
            }
        }
        return t;
    }

    std::size_t boundary_edges() const {
        std::size_t n=0;
        for(const auto& [e,faces]:incident_faces){(void)e;if(faces.size()==1U)++n;}
        return n;
    }
    std::size_t nonmanifold_edges() const {
        std::size_t n=0;
        for(const auto& [e,faces]:incident_faces){(void)e;if(faces.size()>2U)++n;}
        return n;
    }
    long long euler_characteristic(std::size_t face_count) const {
        return static_cast<long long>(used_vertices.size())
             - static_cast<long long>(incident_faces.size())
             + static_cast<long long>(face_count);
    }
};

struct Quadric {
    std::array<double,16> m{};
    Quadric& operator+=(const Quadric& q){
        for(std::size_t i=0;i<m.size();++i)m[i]+=q.m[i];
        return *this;
    }
};

Quadric plane_quadric(double a,double b,double c,double d){
    const std::array<double,4> p{a,b,c,d};
    Quadric q;
    for(int r=0;r<4;++r) for(int col=0;col<4;++col)
        q.m[static_cast<std::size_t>(r*4+col)]=p[static_cast<std::size_t>(r)]*p[static_cast<std::size_t>(col)];
    return q;
}

double qem_cost(const Quadric& q,V3 p){
    const std::array<double,4> h{p.x,p.y,p.z,1.0};
    double s=0.0;
    for(int r=0;r<4;++r) for(int c=0;c<4;++c)
        s+=h[static_cast<std::size_t>(r)]*q.m[static_cast<std::size_t>(r*4+c)]*h[static_cast<std::size_t>(c)];
    return s;
}

std::vector<Quadric> vertex_quadrics(const std::vector<V3>& v,const std::vector<Tri>& f){
    std::vector<Quadric> q(v.size());
    for(const Tri& t:f){
        for(int i:t) if(static_cast<std::size_t>(i)>=v.size()) throw std::runtime_error("triangle index out of range");
        const V3 n=normalize(cross(v[static_cast<std::size_t>(t[1])]-v[static_cast<std::size_t>(t[0])],
                                   v[static_cast<std::size_t>(t[2])]-v[static_cast<std::size_t>(t[0])]));
        const double d=-dot(n,v[static_cast<std::size_t>(t[0])]);
        const Quadric fq=plane_quadric(n.x,n.y,n.z,d);
        for(int i:t) q[static_cast<std::size_t>(i)]+=fq;
    }
    return q;
}

bool triangle_would_degenerate(const Tri& t,int keep,int remove,const std::vector<V3>& vertices,V3 new_pos){
    std::array<V3,3> p{};
    for(int k=0;k<3;++k){
        const int idx=t[static_cast<std::size_t>(k)];
        p[static_cast<std::size_t>(k)]=(idx==keep||idx==remove)?new_pos:vertices[static_cast<std::size_t>(idx)];
    }
    return length(cross(p[1]-p[0],p[2]-p[0]))<1.0e-10;
}

void tests(){
    {
        // Two triangles form one square disk: V=4,E=5,F=2 -> chi=1, boundary=4.
        const std::vector<Tri> f{{0,1,2},{0,2,3}};
        const Topology t=Topology::build(f);
        assert(t.boundary_edges()==4U);
        assert(t.nonmanifold_edges()==0U);
        assert(t.euler_characteristic(f.size())==1);
    }
    {
        // Closed tetrahedron: V=4,E=6,F=4 -> chi=2.
        const std::vector<Tri> f{{0,2,1},{0,1,3},{1,2,3},{2,0,3}};
        const Topology t=Topology::build(f);
        assert(t.boundary_edges()==0U);
        assert(t.nonmanifold_edges()==0U);
        assert(t.euler_characteristic(f.size())==2);
    }
    {
        // Three faces sharing edge (0,1) make that edge non-manifold.
        const std::vector<Tri> f{{0,1,2},{1,0,3},{0,1,4}};
        const Topology t=Topology::build(f);
        assert(t.nonmanifold_edges()==1U);
    }
    {
        const std::vector<V3> v{{0,0,0},{1,0,0},{1,1,0},{0,1,0}};
        const std::vector<Tri> f{{0,1,2},{0,2,3}};
        auto q=vertex_quadrics(v,f);
        Quadric edgeq=q[0]; edgeq+=q[2];
        // Every point on z=0 has zero distance to these face planes.
        assert(std::abs(qem_cost(edgeq,{0.5,0.5,0.0}))<1.0e-12);
        assert(qem_cost(edgeq,{0.5,0.5,1.0})>1.0);
        // Collapsing the diagonal to its midpoint destroys both triangles.
        assert(triangle_would_degenerate(f[0],0,2,v,{0.5,0.5,0.0}));
        assert(triangle_would_degenerate(f[1],0,2,v,{0.5,0.5,0.0}));
    }
}

} // namespace allmight

int main(){
    allmight::tests();
    std::cout<<"mesh_topology_qem_lab: PASS\n";
    return 0;
}
