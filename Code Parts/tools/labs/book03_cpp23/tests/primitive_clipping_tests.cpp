#include "book03/primitive_clipping.hpp"
#include <iostream>
#include <random>
#include <string_view>
namespace {
using namespace book03;
void check(bool b,std::string_view m){if(!b)throw std::runtime_error(std::string(m));}
void near(double a,double b){check(std::isfinite(a)&&std::abs(a-b)<1e-8*(1+std::abs(b)),"numeric witness");}
ProcessedVertex v(double x,double y,double z,double w,double u=0,double t=0){
    ProcessedVertex p;p.clip={x,y,z,w};p.varying.world={x,y,z};
    p.varying.uv=UV2{u,t};p.varying.world_normal=Vector3{1-u,u,0};return p;
}
void lines(){
    Window2 w{0,0,10,10};
    auto a=cohen_sutherland(w,{{-5,5},{15,5}});
    auto b=liang_barsky(w,{{-5,5},{15,5}});
    for(auto x:{a,b}){check(x.has_value(),"crossing");near(x->a.u,0);near(x->b.u,10);near(x->a.v,5);}
    check(!liang_barsky(w,{{-2,0},{-1,10}}),"outside parallel");
    check(!cohen_sutherland(w,{{-2,0},{-1,10}}),"outside parallel");
    auto point=liang_barsky(w,{{-1,1},{1,-1}});
    check(point.has_value(),"closed corner");near(point->a.u,0);near(point->b.v,0);
    std::mt19937_64 rng(170017);
    auto s=[&]{return static_cast<double>(rng()>>11)*0x1.0p-53;};
    for(int i=0;i<4000;++i){
        Line2 l{{30*s()-10,30*s()-10},{30*s()-10,30*s()-10}};
        auto a=cohen_sutherland(w,l),b=liang_barsky(w,l);
        check(a.has_value()==b.has_value(),"line agreement");
        if(a){near(a->a.u,b->a.u);near(a->a.v,b->a.v);near(a->b.u,b->b.u);near(a->b.v,b->b.v);}
    }
}
void triangles(){
    std::array a{v(-2,0,.5,1,0,0),v(0,-.5,.5,1,1,0),v(0,.5,.5,1,0,1)};
    auto p=clip_triangle(a);
    check(p.size()==4,"one outside yields quad");check(fan_triangulate(p).size()==2,"quad fan");
    near(p[0].clip.x,-1);near(p[0].clip.y,-.25);near(p[0].varying.uv->u,.5);
    near(p[0].varying.world_normal->x,.5);near(p[0].varying.world_normal->y,.5);
    near(p[3].clip.x,-1);near(p[3].clip.y,.25);near(p[3].varying.uv->v,.5);
    for(const auto& q:p){
        check(classify_clip(q.clip).inside(),"inside");
        near(q.source_weights[0]+q.source_weights[1]+q.source_weights[2],1);
        near(q.varying.uv->u,q.source_weights[1]);near(q.varying.uv->v,q.source_weights[2]);
        (void)to_screen_vertex(q,{});
    }
    auto reversed=clip_triangle(std::array{a[1],a[0],a[2]});
    bool found=false;for(auto& q:reversed)if(q.clip.x==p[0].clip.x&&q.clip.y==p[0].clip.y)found=true;
    check(found,"reversed edge consistent");
    check(clip_triangle(std::array{v(-2,0,.5,1),v(-3,1,.5,1),v(-4,-1,.5,1)}).empty(),"fully outside");
    auto on=clip_triangle(std::array{v(-1,0,.5,1),v(0,-.5,.5,1),v(0,.5,.5,1)});
    check(on.size()==3,"boundary does not duplicate");
    auto z=clip_triangle(std::array{v(0,0,-1,2),v(1,0,1,2),v(0,1,1,2)});
    check(z.size()==4,"depth plane crossing");
    for(auto& q:z)check(q.clip.z>=0,"z clipped");
    // Real P14 projection, with a point behind the camera.
    VertexStage stage({}, {},perspective_matrix({}, {1,20}),{});
    std::array<ProcessedVertex,3> in;
    std::array<Point3,3> positions{{{0,0,1},{-1,-1,-4},{1,-1,-4}}};
    for(int i=0;i<3;++i){ObjRenderingVertex x{};x.position=positions[i];in[i]=stage.process(x);}
    auto visible=clip_triangle(in);check(visible.size()>=3,"behind-camera triangle survives");
    for(auto& q:visible)check(q.clip.w>0&&classify_clip(q.clip).inside(),"positive w");
}
void seeded_triangles(){
    std::mt19937_64 rng(170018);
    auto s=[&]{return static_cast<double>(rng()>>11)*0x1.0p-53;};
    // Isolate a single clipping plane so drift from intersecting planes
    // cannot obscure the analytic reconstruction test.
    for(int i=0;i<2000;++i){
        std::array a{v(-1-2*s(),0,.5,1,0,0),v(.2+s()*.5,-.5,.5,1,1,0),v(.2+s()*.5,.5,.5,1,0,1)};
        auto p=clip_triangle(a);check(p.size()==4,"quad");
        for(auto& q:p){
            const auto b=q.source_weights;
            near(q.clip.x,b[0]*a[0].clip.x+b[1]*a[1].clip.x+b[2]*a[2].clip.x);
            near(q.clip.y,b[0]*a[0].clip.y+b[1]*a[1].clip.y+b[2]*a[2].clip.y);
            near(q.varying.uv->u,b[1]);near(q.varying.uv->v,b[2]);near(b[0]+b[1]+b[2],1);
        }
    }
}
}
int main(){try{lines();triangles();seeded_triangles();std::cout<<"Book03/P17 clipping contracts: PASS\n";return 0;}
catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
