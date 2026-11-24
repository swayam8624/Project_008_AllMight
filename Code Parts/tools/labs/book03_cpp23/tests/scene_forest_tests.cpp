// P11 independent scene contracts. Expected coordinates are derived by hand;
// no comparison against a second copy of the implementation.
#include "book03/scene_forest.hpp"
#include <cmath>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
using namespace book03;
void require(bool x,std::string_view m) {
    if(!x)throw std::runtime_error(std::string(m));
}
void near(double a,double b,double tolerance=1e-10) {
    require(std::isfinite(a)&&std::abs(a-b)<=tolerance*(1+std::abs(b)),
            "numeric scene witness");
}
void same(Point3 a,Point3 b) {near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
template<class E,class F> void reject(F f) {
    try{f();}catch(const E&){return;}
    throw std::runtime_error("expected scene validation failure");
}
void order_and_inverse() {
    const auto T=scene_translate({4,0,0});
    const auto R=scene_rotate_y(std::numbers::pi/2);
    const auto S=scene_scale({2,3,4});
    same((T*R*S).point({1,2,3}),{16,6,-2});
    same((R*T*S).point({1,2,3}),{12,6,-6});
    const auto map=T*R*S;
    same(map.inverse().point(map.point({1,2,3})),{1,2,3});
    same((map*map.inverse()).point({-4,3,2}),{-4,3,2});
    // A sheared, non-diagonal matrix provides a separate inverse witness.
    const SceneAffine shear{{{1,0,0},{2,1,0},{0,3,1}},{1,2,3}};
    same(shear.point({1,2,3}),{6,13,6});
    same(shear.inverse().point({6,13,6}),{1,2,3});
    for(int i=-9;i<=9;++i)for(int j=-7;j<=7;++j) {
        const Point3 p{0.13*i,0.19*j,0.3*(i-j)};
        same(shear.inverse().point(shear.point(p)),p);
    }
}
void normals_and_reflections() {
    const auto S=scene_scale({2,1,3});
    const Vector3 t{1,1,0},n=unit({-1,1,0});
    const auto t2=S.tangent(t),n2=S.normal(n);
    near(dot(t2,n2),0);near(magnitude(n2),1);
    require(std::abs(dot(t2,unit(S.tangent(n))))>0.5,
            "normal must not use ordinary tangent transform");
    const auto reflection=scene_scale({-1,1,1});
    const auto a=reflection.tangent({1,0,0});
    const auto b=reflection.tangent({0,1,0});
    require(reflection.linear.determinant()<0,"reflection parity");
    near(dot(unit(cross(a,b)),reflection.normal({0,0,1})),-1);
    reject<std::domain_error>([]{
        (void)scene_scale({1,0,1}).normal({0,0,1});});
    reject<std::domain_error>([]{
        (void)scene_scale({1,1e-15,1}).inverse();});
}
void shared_assets_and_bounds() {
    const auto s=five_tree_world();
    const auto e=s.evaluate();
    require(s.assets.size()==1&&s.assets[0].positions.size()==7&&
            s.assets[0].triangles.size()==3,"only one tree asset");
    require(e.instances.size()==5&&e.order.size()==7,"five instances");
    for(const auto& i:e.instances) require(i.asset==0,"shared asset identity");
    same(e.world[2].point({0,0,0}),{-3,0,0});
    same(e.world[3].point({0,0,0}),{-1,0,0});
    same(e.world[4].point({0,0,0}),{3,0,0});
    same(e.world[5].point({0,2.5,0}),{5,3.25,0});
    // Instances are emitted in topological order, not in node ID order.
    const auto a=std::find_if(e.instances.begin(),e.instances.end(),
        [](const SceneInstance& i){return i.node==2;});
    require(a!=e.instances.end(),"tree A exists");
    same(a->bounds.min,{-3.8,0,0});
    same(a->bounds.max,{-2.2,2.5,0});
    // Parent may occur AFTER its child in the input array.
    SceneForest reversed;
    reversed.nodes={{"child",1,{},scene_translate({2,0,0})},
                    {"root",{},{},scene_translate({5,0,0})}};
    const auto r=reversed.evaluate();
    require(r.order==std::vector<std::size_t>{1,0},"topological order");
    same(r.world[0].point({0,0,0}),{7,0,0});
}
void reparent_and_reject() {
    auto s=five_tree_world();
    const auto before=s.evaluate().world[3];
    s.reparent(3,1,true);
    const auto after=s.evaluate().world[3];
    for(const auto p:s.assets[0].positions)same(before.point(p),after.point(p));
    same(s.nodes[3].local.point({0,0,0}),{-4,0,0});
    s.reparent(3,0,false);
    same(s.evaluate().world[3].point({0,0,0}),{-7,0,0});
    // 2 is a child of 0. Parenting 0 under 2 would form a cycle.
    reject<std::invalid_argument>([&]{s.reparent(0,2,false);});
    require(!s.nodes[0].parent,"failed mutation restored root");
    (void)s.evaluate();
    auto bad=s;bad.nodes[0].parent=999;
    reject<std::out_of_range>([&]{(void)bad.evaluate();});
    bad=s;bad.nodes[2].asset=9;
    reject<std::out_of_range>([&]{(void)bad.evaluate();});
    bad=s;bad.nodes[0].parent=0;
    reject<std::invalid_argument>([&]{(void)bad.evaluate();});
    bad=s;bad.nodes[1].local=scene_scale({1,0,1});
    reject<std::domain_error>([&]{bad.reparent(2,1,true);});
    require(bad.nodes[2].parent==0,"singular parent rejected before mutation");
    SceneForest deep;
    for(std::size_t i=0;i<5000;++i)
        deep.nodes.push_back({"node",i?std::optional<std::size_t>{i-1}:
            std::nullopt,{},scene_translate({1,0,0})});
    same(deep.evaluate().world.back().point({0,0,0}),{5000,0,0});
}
}
int main() {
    try{order_and_inverse();normals_and_reflections();
        shared_assets_and_bounds();reparent_and_reject();
        std::cout<<"Book03/P11 scene forest contracts: PASS\n";return 0;
    }catch(const std::exception& e){
        std::cerr<<e.what()<<'\n';return 1;
    }
}
