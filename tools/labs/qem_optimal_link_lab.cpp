// PROJECTALLMIGHT QEM continuation: solve the 3x3 optimal position and test the triangle-mesh link condition.
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <iterator>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

struct V3{ double x{},y{},z{}; };
V3 operator-(V3 a,V3 b){ return {a.x-b.x,a.y-b.y,a.z-b.z}; }
V3 operator*(V3 a,double s){ return {a.x*s,a.y*s,a.z*s}; }
double dot(V3 a,V3 b){ return a.x*b.x+a.y*b.y+a.z*b.z; }
V3 cross(V3 a,V3 b){ return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
double norm(V3 v){ return std::sqrt(dot(v,v)); }

using M4=std::array<double,16>;

M4 add(M4 a,const M4& b){ for(std::size_t i=0;i<16;++i)a[i]+=b[i]; return a; }

M4 plane_quadric(V3 a,V3 b,V3 c){
    V3 n=cross(b-a,c-a);
    const double l=norm(n);
    require(l>1e-12,"degenerate face");
    n=n*(1.0/l);
    const double d=-dot(n,a);
    const std::array<double,4> p{n.x,n.y,n.z,d};
    M4 q{};
    for(int r=0;r<4;++r)for(int col=0;col<4;++col)
        q[static_cast<std::size_t>(4*r+col)]=p[r]*p[col];
    return q;
}

bool solve_optimal(const M4& q,V3& out){
    double a[3][4]{
        {q[0],q[1],q[2],-q[3]},
        {q[4],q[5],q[6],-q[7]},
        {q[8],q[9],q[10],-q[11]},
    };
    for(int col=0;col<3;++col){
        int pivot=col;
        for(int r=col+1;r<3;++r) if(std::fabs(a[r][col])>std::fabs(a[pivot][col])) pivot=r;
        if(std::fabs(a[pivot][col])<1e-12) return false;
        if(pivot!=col) for(int c=col;c<4;++c) std::swap(a[pivot][c],a[col][c]);
        const double inv=1.0/a[col][col];
        for(int c=col;c<4;++c)a[col][c]*=inv;
        for(int r=0;r<3;++r){
            if(r==col)continue;
            const double f=a[r][col];
            for(int c=col;c<4;++c)a[r][c]-=f*a[col][c];
        }
    }
    out={a[0][3],a[1][3],a[2][3]};
    return true;
}

double evaluate(const M4& q,V3 v){
    const std::array<double,4> h{v.x,v.y,v.z,1};
    double sum=0;
    for(int r=0;r<4;++r)for(int c=0;c<4;++c)
        sum+=h[r]*q[static_cast<std::size_t>(4*r+c)]*h[c];
    return sum;
}

struct Tri{ int a{},b{},c{}; };

std::set<int> one_ring(const std::vector<Tri>& tris,int v){
    std::set<int> n;
    for(const Tri& t:tris){
        const int ids[3]{t.a,t.b,t.c};
        bool has=false;
        for(int x:ids) if(x==v) has=true;
        if(!has) continue;
        for(int x:ids) if(x!=v) n.insert(x);
    }
    return n;
}

std::set<int> opposite_vertices(const std::vector<Tri>& tris,int u,int v){
    std::set<int> opp;
    for(const Tri& t:tris){
        const int ids[3]{t.a,t.b,t.c};
        bool hu=false,hv=false;
        for(int x:ids){ hu|=x==u; hv|=x==v; }
        if(hu&&hv) for(int x:ids) if(x!=u&&x!=v) opp.insert(x);
    }
    return opp;
}

bool manifold_link_condition(const std::vector<Tri>& tris,int u,int v){
    auto nu=one_ring(tris,u);
    auto nv=one_ring(tris,v);
    nu.erase(v); nv.erase(u);
    std::set<int> intersection;
    std::set_intersection(nu.begin(),nu.end(),nv.begin(),nv.end(),
                          std::inserter(intersection,intersection.begin()));
    const auto opposite=opposite_vertices(tris,u,v);
    // An ordinary triangle-mesh edge has one incident face on a boundary or two in the interior.
    // More than two incident faces is non-manifold and must not be accepted by this collapse gate.
    if(opposite.empty() || opposite.size()>2) return false;
    return intersection==opposite;
}

void test_optimal_point(){
    // Three orthogonal planes x=1, y=2, z=3 intersect at a unique minimizer.
    const M4 qx=plane_quadric({1,0,0},{1,1,0},{1,0,1});
    const M4 qy=plane_quadric({0,2,0},{0,2,1},{1,2,0});
    const M4 qz=plane_quadric({0,0,3},{1,0,3},{0,1,3});
    const M4 q=add(add(qx,qy),qz);
    V3 p{};
    require(solve_optimal(q,p),"full-rank quadric solve reported singular");
    require(std::fabs(p.x-1)<1e-12&&std::fabs(p.y-2)<1e-12&&std::fabs(p.z-3)<1e-12,
            "QEM optimal point does not match three-plane intersection");
    require(evaluate(q,p)<1e-20,"optimal three-plane point should have zero error");

    const M4 single=qx;
    require(!solve_optimal(single,p),"rank-deficient single-plane quadric should require fallback");
}

void test_link_condition(){
    // Closed tetrahedron: every edge has exactly the two opposite/common one-ring vertices required.
    const std::vector<Tri> tetra{{0,1,2},{0,3,1},{0,2,3},{1,3,2}};
    require(manifold_link_condition(tetra,0,1),"valid tetrahedron edge failed link condition");

    // Non-manifold edge 0-1 has three incident triangles, so the common neighbor set has
    // three opposite vertices. For a 2-manifold interior edge we explicitly require two.
    const std::vector<Tri> nonmanifold{{0,1,2},{0,3,1},{0,1,4}};
    const auto opp=opposite_vertices(nonmanifold,0,1);
    require(opp.size()==3,"non-manifold fixture did not create three incident faces");
    require(!manifold_link_condition(nonmanifold,0,1),
            "non-manifold edge incorrectly accepted as ordinary manifold edge");
}

} // namespace

int main(){
    try{
        test_optimal_point();
        test_link_condition();
        std::cout
            <<"qem_optimal_link_lab: PASS\n"
            <<"proved: pivoted 3x3 QEM minimizer solve, singular fallback detection, zero-error three-plane optimum, one-ring/opposite-set link-condition machinery, non-manifold incident-edge detection\n"
            <<"not proved: boundary-specific link conditions, attribute quadrics, optimal fallback selection among endpoints/midpoint, priority-queue simplifier updates, progressive mesh encoding, or production mesh parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"qem_optimal_link_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
