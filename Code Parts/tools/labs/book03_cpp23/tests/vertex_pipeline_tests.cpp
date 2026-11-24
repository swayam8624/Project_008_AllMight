// Independent hand witnesses and deterministic project/interpolate crosschecks.
#include "book03/vertex_pipeline.hpp"
#include <iostream>
#include <random>
#include <sstream>
#include <string_view>
namespace {
using namespace book03;
void require(bool b,std::string_view m){if(!b)throw std::runtime_error(std::string(m));}
void near(double a,double b,double e=1e-9){require(std::isfinite(a)&&std::abs(a-b)<=e*(1+std::abs(b)),"vertex numeric witness");}
void same(Point3 a,Point3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
template<class E,class F>void reject(F f){try{f();}catch(const E&){return;}throw std::runtime_error("expected rejection");}
VertexStage stage(SceneAffine model={},RigidPose pose={}){
    return {model,pose,perspective_matrix({}, {1,20}),{}};
}
ObjAsset::RenderingMesh witness(){
    std::istringstream input("v 0 0 -2\nv 2 0 -4\nv 0 4 -8\n"
        "vt 0 0\nvt 1 0\nvt 0 1\nvn 0 0 1\nf 1/1/1 2/2/1 3/3/1\n");
    return import_obj(input).to_rendering_mesh();
}
std::array<ProcessedVertex,3> first(const VertexDraw& d){
    const auto ids=d.triangles.at(0).vertex;
    return {*d.vertices.at(ids[0]),*d.vertices.at(ids[1]),*d.vertices.at(ids[2])};
}
void analytic(){
    auto s=stage();auto d=s.draw(witness());auto v=first(d);
    require(d.invocations==3,"one invocation per referenced ID");
    near(v[0].clip.w,2);near(v[1].clip.w,4);near(v[2].clip.w,8);
    near(v[0].screen->image.u,320);near(v[1].screen->image.u,520);
    near(v[2].screen->image.v,40);
    auto l=screen_barycentrics(v,{(320.+520+320)/3,(240.+240+40)/3});
    for(auto x:l)near(x,1./3);
    auto p=interpolate_inside_triangle(v,l);
    near(p.surface_weights[0],4./7);near(p.surface_weights[1],2./7);near(p.surface_weights[2],1./7);
    near(p.varying.uv->u,2./7);near(p.varying.uv->v,1./7);
    same(p.varying.world,{4./7,4./7,-24./7});
    near(p.depth,85./114);
    near(p.varying.world_normal->z,1);
    const auto image=PinholeCamera{}.project(p.varying.world).image;
    near(image.u,(320.+520+320)/3);near(image.v,(240.+240+40)/3);
    // Screen-affine UV would produce (1/3,1/3), not this surface point.
    require(std::abs(p.varying.uv->v-1./3)>.1,"perspective correction matters");
    auto end=interpolate_inside_triangle(v,{0,1,0});
    same(end.varying.world,v[1].varying.world);
    near(end.varying.uv->u,1);
    v[1].varying.uv.reset();
    reject<std::invalid_argument>([&]{(void)interpolate_inside_triangle(v,{.2,.3,.5});});
}
void seams_and_instances(){
    std::istringstream input("v 0 0 -4\nv 1 0 -4\nv 1 1 -4\nv 0 1 -4\n"
        "vt 0 0\nvt 1 0\nvt 1 1\nvt 0 1\nvt .2 .2\n"
        "f 1/1 2/2 3/3\nf 1/5 3/3 4/4\n");
    auto asset=import_obj(input);auto m=asset.to_rendering_mesh();
    require(asset.topological_mesh().positions.size()==4,"four geometric positions");
    require(m.vertices.size()==5,"five full tuples");
    auto s=stage();auto d=s.draw(m);
    require(d.invocations==5&&d.triangles.size()==2,"shared tuple reused");
    auto a=*d.vertices[0],b=*d.vertices[3];
    same(a.varying.world,b.varying.world);
    require(a.source.position==b.source.position&&a.varying.uv->u!=b.varying.uv->u,"seam preserved");
    auto moved=stage(scene_translate({1,0,0}));
    auto other=moved.draw(m);
    near(other.vertices[0]->screen->image.u-a.screen->image.u,100);
    m.triangles[0].vertex[0]=999;
    reject<std::out_of_range>([&]{(void)s.draw(m);});
}
void frames_and_rejections(){
    const auto model=scene_translate({1,2,-6})*scene_scale({2,1,3});
    const RigidPose pose{Rotation3::axis_angle({0,1,0},std::numbers::pi/2),{3,2,-6}};
    auto s=stage(model,pose);
    ObjRenderingVertex a{};a.position={0,0,0};a.normal=Vector3{1,1,0};
    auto v=s.process(a);
    same(v.varying.world,{1,2,-6});same(v.camera,{0,0,-2});
    near(v.screen->image.u,320);near(v.screen->image.v,240);
    near(dot(*v.varying.world_normal,{2,-1,0}),0);
    auto singular=stage(scene_scale({1,0,1}));
    reject<std::domain_error>([&]{(void)singular.process(a);});
    a.normal.reset();(void)singular.process(a);
    auto normal=stage();a.position={0,0,1};a.uv=UV2{.3,.7};
    auto behind=normal.process(a);
    require(!behind.screen&&behind.clip.w<0&&behind.varying.uv.has_value(),"behind payload retained");
    a.position={0,0,-.5};auto near_plane=normal.process(a);
    require(!near_plane.screen&&near_plane.clip.w>0,"near outside retained");
    auto tri=first(normal.draw(witness()));tri[0]=behind;
    reject<std::invalid_argument>([&]{(void)interpolate_inside_triangle(tri,{.3,.3,.4});});
}
void seeded(){
    auto s=stage();auto v=first(s.draw(witness()));
    std::mt19937_64 rng(160016);
    auto random=[&]{return static_cast<double>(rng()>>11)*0x1.0p-53;};
    for(int i=0;i<3000;++i){
        double a=random(),b=random();if(a+b>1){a=1-a;b=1-b;}
        std::array<double,3> beta{1-a-b,a,b};
        // Start with known SURFACE weights, independently project the result.
        Point3 p{2*a,4*b,-(2*beta[0]+4*a+8*b)};
        const auto image=PinholeCamera{}.project(p).image;
        const auto q=interpolate_inside_triangle(v,screen_barycentrics(v,image));
        same(q.varying.world,p);near(q.varying.uv->u,a);near(q.varying.uv->v,b);
        near(q.depth,DepthMap{1,20}.perspective_depth(-p.z));
    }
}
}
int main(){try{analytic();seams_and_instances();frames_and_rejections();seeded();
    std::cout<<"Book03/P16 indexed vertex and varying contracts: PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
