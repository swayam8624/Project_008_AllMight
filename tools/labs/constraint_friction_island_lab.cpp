// PROJECTALLMIGHT contact-solver extension: union-find islands, normal impulses and Coulomb friction clamp.
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

struct DSU{
    std::vector<int> p,rank;
    explicit DSU(int n):p(n),rank(n,0){ for(int i=0;i<n;++i)p[i]=i; }
    int find(int x){ return p[x]==x?x:(p[x]=find(p[x])); }
    void unite(int a,int b){ a=find(a); b=find(b); if(a==b)return; if(rank[a]<rank[b])std::swap(a,b); p[b]=a; if(rank[a]==rank[b])++rank[a]; }
};

struct Body{ double inv_mass{},vn{},vt{}; };
struct Contact{ int a{},b{}; double mu{}; };

double solve_normal(Body& a,Body& b){
    const double rel=b.vn-a.vn;
    if(rel>=0) return 0.0;
    const double denom=a.inv_mass+b.inv_mass;
    require(denom>0,"contact has no movable body");
    const double j=-rel/denom;
    a.vn-=j*a.inv_mass;
    b.vn+=j*b.inv_mass;
    return j;
}

double solve_friction(Body& a,Body& b,double mu,double normal_impulse){
    require(mu>=0&&normal_impulse>=0,"invalid Coulomb inputs");
    const double rel=b.vt-a.vt;
    const double denom=a.inv_mass+b.inv_mass;
    if(denom<=0) return 0;
    const double unconstrained=-rel/denom;
    const double limit=mu*normal_impulse;
    const double jt=std::clamp(unconstrained,-limit,limit);
    a.vt-=jt*a.inv_mass;
    b.vt+=jt*b.inv_mass;
    return jt;
}

void test_islands(){
    DSU d(6);
    for(const auto [a,b]: {std::pair{0,1},std::pair{1,2},std::pair{3,4}}) d.unite(a,b);
    require(d.find(0)==d.find(2),"connected contact chain not in one island");
    require(d.find(0)!=d.find(3),"disconnected groups merged into one island");
    require(d.find(5)!=d.find(0)&&d.find(5)!=d.find(3),"isolated body incorrectly merged");
}

void test_ground_contact_friction(){
    Body ground{0,0,0};
    Body body{1,-3,4};
    const double jn=solve_normal(ground,body);
    require(std::fabs(jn-3.0)<1e-12&&std::fabs(body.vn)<1e-12,"normal impulse did not stop approach");
    const double jt=solve_friction(ground,body,0.5,jn);
    require(std::fabs(jt+1.5)<1e-12,"friction impulse should clamp to mu*normal");
    require(std::fabs(body.vt-2.5)<1e-12,"friction velocity update wrong");
}

void test_two_body_stack_convergence(){
    Body ground{0,0,0};
    Body lower{1,-1,0};
    Body upper{1,-1,0};
    for(int iteration=0;iteration<24;++iteration){
        (void)solve_normal(ground,lower);
        (void)solve_normal(lower,upper);
    }
    require(lower.vn>-1e-6&&upper.vn>-1e-6,"sequential impulses failed to remove downward stack velocity");
    require(std::fabs(lower.vn-upper.vn)<1e-5,"stack bodies did not converge to compatible normal velocities");
}

} // namespace

int main(){
    try{
        test_islands();
        test_ground_contact_friction();
        test_two_body_stack_convergence();
        std::cout<<"constraint_friction_island_lab: PASS\n"
                 <<"proved: contact-island connectivity, unilateral normal impulse, Coulomb tangent clamp, simple two-body stack convergence under sequential impulses\n"
                 <<"not proved: angular Jacobians, restitution thresholds, multi-direction friction cones, warm-start scaling, shock propagation, sleeping, or production 3D solver parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"constraint_friction_island_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
