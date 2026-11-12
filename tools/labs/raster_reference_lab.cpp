// PROJECTALLMIGHT raster reference: edge ownership, barycentrics, perspective correction, depth.
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }
bool near(float a,float b,float e=1e-5F){ return std::fabs(a-b)<=e; }

struct V2 { float x=0,y=0; };
struct Vertex { V2 p; float inv_w=1; float attr=0; float depth=0; };

float edge(V2 a,V2 b,V2 p){
    return (p.x-a.x)*(b.y-a.y)-(p.y-a.y)*(b.x-a.x);
}

bool top_left(V2 a,V2 b){
    const float dy=b.y-a.y;
    const float dx=b.x-a.x;
    return dy>0.0F || (dy==0.0F && dx<0.0F);
}

struct SampleResult {
    bool covered=false;
    std::array<float,3> lambda{};
    float perspective_attr=0;
    float depth=0;
};

SampleResult sample_triangle(std::array<Vertex,3> v,V2 p){
    // Normalize winding so positive signed area means the edge tests share one convention.
    float area=edge(v[0].p,v[1].p,v[2].p);
    require(std::fabs(area)>1e-8F,"degenerate triangle");
    if(area<0){
        std::swap(v[1],v[2]);
        area=-area;
    }

    const float e0=edge(v[1].p,v[2].p,p);
    const float e1=edge(v[2].p,v[0].p,p);
    const float e2=edge(v[0].p,v[1].p,p);

    auto accepts=[](float e,V2 a,V2 b){
        return e>0.0F || (e==0.0F && top_left(a,b));
    };
    if(!accepts(e0,v[1].p,v[2].p) ||
       !accepts(e1,v[2].p,v[0].p) ||
       !accepts(e2,v[0].p,v[1].p)) return {};

    SampleResult r;
    r.covered=true;
    r.lambda={e0/area,e1/area,e2/area};

    float denom=0.0F;
    float numer=0.0F;
    float z=0.0F;
    for(int i=0;i<3;++i){
        denom += r.lambda[i]*v[i].inv_w;
        numer += r.lambda[i]*v[i].attr*v[i].inv_w;
        z += r.lambda[i]*v[i].depth;
    }
    require(std::fabs(denom)>1e-12F,"perspective denominator vanished");
    r.perspective_attr=numer/denom;
    r.depth=z;
    return r;
}

void test_shared_edge_has_single_owner(){
    // Square split along the diagonal. Sample centers intentionally include a point on the diagonal.
    std::array<Vertex,3> a{{{{0,0},1,0,0.2F},{{2,0},1,0,0.2F},{{2,2},1,0,0.2F}}};
    std::array<Vertex,3> b{{{{0,0},1,0,0.3F},{{2,2},1,0,0.3F},{{0,2},1,0,0.3F}}};

    const std::vector<V2> samples{{0.5F,0.5F},{1.0F,1.0F},{1.5F,1.5F}};
    for(V2 p:samples){
        const bool ca=sample_triangle(a,p).covered;
        const bool cb=sample_triangle(b,p).covered;
        require(ca!=cb,"shared edge must have exactly one owner under top-left rule");
    }
}

void test_perspective_interpolation(){
    std::array<Vertex,3> tri{{
        {{0,0},1.0F,0.0F,0.1F},
        {{2,0},0.5F,10.0F,0.2F},
        {{0,2},0.25F,20.0F,0.3F}
    }};
    const V2 p{0.5F,0.5F};
    const auto r=sample_triangle(tri,p);
    require(r.covered,"test point should be covered");

    const float affine=r.lambda[0]*0.0F+r.lambda[1]*10.0F+r.lambda[2]*20.0F;
    require(!near(affine,r.perspective_attr,1e-3F),
            "chosen test must distinguish affine from perspective interpolation");

    float denom=0,numer=0;
    for(int i=0;i<3;++i){
        denom+=r.lambda[i]*tri[i].inv_w;
        numer+=r.lambda[i]*tri[i].attr*tri[i].inv_w;
    }
    require(near(r.perspective_attr,numer/denom),"perspective attribute equation mismatch");
}

void test_depth_order(){
    std::array<Vertex,3> near_t{{{{0,0},1,0,0.2F},{{2,0},1,0,0.2F},{{0,2},1,0,0.2F}}};
    std::array<Vertex,3> far_t{{{{0,0},1,0,0.8F},{{2,0},1,0,0.8F},{{0,2},1,0,0.8F}}};
    const V2 p{0.25F,0.25F};
    auto n=sample_triangle(near_t,p);
    auto f=sample_triangle(far_t,p);
    require(n.covered&&f.covered,"depth test setup uncovered");
    require(n.depth<f.depth,"standard less-depth convention should select near triangle");
}

} // namespace

int main(){
    try{
        test_shared_edge_has_single_owner();
        test_perspective_interpolation();
        test_depth_order();
        std::cout<<"raster_reference_lab: PASS\n"
                 <<"proved: top-left shared-edge ownership, barycentric coverage, perspective-correct attribute interpolation, depth ordering\n"
                 <<"not proved: homogeneous clipping, MSAA sample patterns, stencil/blend state, derivatives, conservative rasterization, or GPU parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"raster_reference_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
