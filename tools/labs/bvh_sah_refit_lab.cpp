// PROJECTALLMIGHT spatial refinement: binned-style SAH split ingredients, Morton ordering and refit invariants.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

struct V3{ float x{},y{},z{}; float operator[](int a)const{return a==0?x:(a==1?y:z);} };
struct Aabb{
    V3 lo{ std::numeric_limits<float>::infinity(),std::numeric_limits<float>::infinity(),std::numeric_limits<float>::infinity()};
    V3 hi{-std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()};
};
Aabb merge(Aabb a,const Aabb& b){
    a.lo.x=std::min(a.lo.x,b.lo.x); a.lo.y=std::min(a.lo.y,b.lo.y); a.lo.z=std::min(a.lo.z,b.lo.z);
    a.hi.x=std::max(a.hi.x,b.hi.x); a.hi.y=std::max(a.hi.y,b.hi.y); a.hi.z=std::max(a.hi.z,b.hi.z);
    return a;
}
float area(const Aabb& b){
    const float x=std::max(0.0F,b.hi.x-b.lo.x),y=std::max(0.0F,b.hi.y-b.lo.y),z=std::max(0.0F,b.hi.z-b.lo.z);
    return 2.0F*(x*y+y*z+z*x);
}
V3 centroid(const Aabb& b){ return {(b.lo.x+b.hi.x)*0.5F,(b.lo.y+b.hi.y)*0.5F,(b.lo.z+b.hi.z)*0.5F}; }

struct Split{ int axis=-1; std::size_t index=0; float cost=std::numeric_limits<float>::infinity(); };

Split best_sah_split(std::vector<Aabb> boxes){
    require(boxes.size()>=2,"SAH split needs two primitives");
    Split best;
    for(int axis=0;axis<3;++axis){
        std::sort(boxes.begin(),boxes.end(),[&](const Aabb& a,const Aabb& b){return centroid(a)[axis]<centroid(b)[axis];});
        std::vector<Aabb> prefix(boxes.size()),suffix(boxes.size());
        prefix[0]=boxes[0];
        for(std::size_t i=1;i<boxes.size();++i) prefix[i]=merge(prefix[i-1],boxes[i]);
        suffix.back()=boxes.back();
        for(std::size_t i=boxes.size()-1;i-->0;) suffix[i]=merge(suffix[i+1],boxes[i]);
        for(std::size_t i=1;i<boxes.size();++i){
            const float c=area(prefix[i-1])*static_cast<float>(i)+
                          area(suffix[i])*static_cast<float>(boxes.size()-i);
            if(c<best.cost){ best={axis,i,c}; }
        }
    }
    return best;
}

std::uint32_t spread10(std::uint32_t x){
    x&=0x3ffu;
    x=(x|(x<<16))&0x030000FFu;
    x=(x|(x<<8 ))&0x0300F00Fu;
    x=(x|(x<<4 ))&0x030C30C3u;
    x=(x|(x<<2 ))&0x09249249u;
    return x;
}
std::uint32_t morton3(std::uint32_t x,std::uint32_t y,std::uint32_t z){
    return spread10(x)|(spread10(y)<<1)|(spread10(z)<<2);
}

void test_sah_cluster_split(){
    std::vector<Aabb> b;
    for(float x:{0.0F,0.2F,0.4F,10.0F,10.2F,10.4F}) b.push_back({{x,0,0},{x+0.1F,1,1}});
    const Split s=best_sah_split(b);
    require(s.axis==0,"separated x clusters should choose x axis");
    require(s.index==3,"SAH should split between the two three-object clusters");
}

void test_morton_order(){
    std::uint32_t prev=0;
    for(std::uint32_t x=0;x<64;++x){
        const auto code=morton3(x,5,7);
        if(x>0) require(code>prev,"Morton code should increase with x when y/z fixed over this range");
        prev=code;
    }
}

void test_refit(){
    Aabb left{{0,0,0},{1,1,1}};
    Aabb right{{2,0,0},{3,1,1}};
    Aabb parent=merge(left,right);
    require(parent.lo.x==0&&parent.hi.x==3,"initial parent merge wrong");
    right={{5,-1,0},{7,2,1}};
    parent=merge(left,right);
    require(parent.lo.x==0&&parent.hi.x==7&&parent.lo.y==-1&&parent.hi.y==2,"parent refit failed after child motion");
    require(area(parent)>=area(left)&&area(parent)>=area(right),"parent surface area must contain child areas");
}

} // namespace

int main(){
    try{
        test_sah_cluster_split();
        test_morton_order();
        test_refit();
        std::cout<<"bvh_sah_refit_lab: PASS\n"
                 <<"proved: surface-area cost ingredients, deterministic best split on separated clusters, Morton bit interleave ordering, parent-bound refit after child motion\n"
                 <<"not proved: full binned SAH builder, LBVH hierarchy construction, rotations/reinsertion, SIMD/GPU traversal, ray packets, or production workload superiority\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"bvh_sah_refit_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
