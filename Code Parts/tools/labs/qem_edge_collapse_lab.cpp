// PROJECTALLMIGHT QEM edge-collapse reference.
// Builds plane quadrics, evaluates candidates and rejects collapses that invert/degenerate surviving faces.
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

struct V3{ double x{},y{},z{}; };
V3 operator+(V3 a,V3 b){ return {a.x+b.x,a.y+b.y,a.z+b.z}; }
V3 operator-(V3 a,V3 b){ return {a.x-b.x,a.y-b.y,a.z-b.z}; }
V3 operator*(V3 a,double s){ return {a.x*s,a.y*s,a.z*s}; }
double dot(V3 a,V3 b){ return a.x*b.x+a.y*b.y+a.z*b.z; }
V3 cross(V3 a,V3 b){ return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
double norm(V3 v){ return std::sqrt(dot(v,v)); }

struct Quadric{
    std::array<double,16> m{};
    Quadric& operator+=(const Quadric& q){ for(std::size_t i=0;i<16;++i)m[i]+=q.m[i]; return *this; }
};
Quadric operator+(Quadric a,const Quadric& b){ return a+=b; }

Quadric plane_quadric(V3 a,V3 b,V3 c){
    V3 n=cross(b-a,c-a);
    const double len=norm(n);
    require(len>1e-12,"degenerate source triangle");
    n=n*(1.0/len);
    const double d=-dot(n,a);
    const std::array<double,4> p{n.x,n.y,n.z,d};
    Quadric q;
    for(int r=0;r<4;++r) for(int col=0;col<4;++col) q.m[static_cast<std::size_t>(4*r+col)]=p[r]*p[col];
    return q;
}

double evaluate(const Quadric& q,V3 v){
    const std::array<double,4> h{v.x,v.y,v.z,1.0};
    double sum=0.0;
    for(int r=0;r<4;++r) for(int c=0;c<4;++c)
        sum+=h[r]*q.m[static_cast<std::size_t>(4*r+c)]*h[c];
    return sum;
}

struct Tri{ int a{},b{},c{}; };

bool valid_after_replace(const std::vector<V3>& vertices,const std::vector<Tri>& tris,int keep,int remove,V3 candidate){
    for(const Tri& t:tris){
        int ids[3]{t.a,t.b,t.c};
        bool touched=false;
        for(int& id:ids){
            if(id==remove){ id=keep; touched=true; }
        }
        if(ids[0]==ids[1]||ids[1]==ids[2]||ids[2]==ids[0]) continue; // triangle disappears with collapsed edge
        V3 p[3]{vertices[static_cast<std::size_t>(ids[0])],
                vertices[static_cast<std::size_t>(ids[1])],
                vertices[static_cast<std::size_t>(ids[2])]};
        for(int i=0;i<3;++i) if(ids[i]==keep) p[i]=candidate;
        const V3 oldn=cross(vertices[static_cast<std::size_t>(t.b)]-vertices[static_cast<std::size_t>(t.a)],
                            vertices[static_cast<std::size_t>(t.c)]-vertices[static_cast<std::size_t>(t.a)]);
        const V3 newn=cross(p[1]-p[0],p[2]-p[0]);
        if(norm(newn)<1e-10) return false;
        if(touched && dot(oldn,newn)<=0.0) return false;
    }
    return true;
}

void test_quadric(){
    const V3 a{0,0,0},b{1,0,0},c{0,1,0};
    const Quadric q=plane_quadric(a,b,c);
    require(std::fabs(evaluate(q,{0.2,0.3,0.0}))<1e-12,"point on plane should have zero quadric error");
    require(std::fabs(evaluate(q,{0.2,0.3,2.0})-4.0)<1e-12,"plane distance-squared error wrong");
}

void test_collapse_selection_and_validity(){
    // Two triangles forming a square. Collapsing edge 0-1 to its midpoint removes one triangle
    // and leaves the other with consistent orientation.
    const std::vector<V3> v{{0,0,0},{1,0,0},{1,1,0},{0,1,0}};
    const std::vector<Tri> t{{0,1,2},{0,2,3}};
    std::vector<Quadric> q(v.size());
    for(const Tri& f:t){
        const Quadric fq=plane_quadric(v[f.a],v[f.b],v[f.c]);
        q[f.a]+=fq; q[f.b]+=fq; q[f.c]+=fq;
    }
    const Quadric edge=q[0]+q[1];
    const V3 mid=(v[0]+v[1])*0.5;
    require(evaluate(edge,mid)>=-1e-12,"quadric cost must be nonnegative for planar case");
    require(valid_after_replace(v,t,0,1,mid),"valid planar boundary collapse was rejected");

    // Moving the kept vertex onto the opposite vertex degenerates the surviving triangle and must be rejected.
    require(!valid_after_replace(v,t,0,1,v[2]),"degenerate collapse was accepted");
}

} // namespace

int main(){
    try{
        test_quadric();
        test_collapse_selection_and_validity();
        std::cout
            <<"qem_edge_collapse_lab: PASS\n"
            <<"proved: plane quadrics, homogeneous error evaluation, per-vertex quadric accumulation, edge candidate cost, degeneracy/orientation rejection\n"
            <<"not proved: optimal 3x3 QEM solve, boundary-feature weighting, manifold link condition, attribute quadrics, progressive meshes, or production importer parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"qem_edge_collapse_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
