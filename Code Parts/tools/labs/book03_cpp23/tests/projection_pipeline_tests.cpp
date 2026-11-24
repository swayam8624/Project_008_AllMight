// P14 — independent analytic tests, not comparison of two copies of a matrix.
#include "book03/projection_pipeline.hpp"
#include <iostream>
#include <random>
#include <string>
#include <string_view>
namespace {
using namespace book03;
void require(bool ok,std::string_view m){if(!ok)throw std::runtime_error(std::string(m));}
void near(double a,double b,double e=1e-10){
    require(std::isfinite(a)&&std::abs(a-b)<=e*(1+std::abs(b)),"projection numerical witness");
}
template<class E,class F>void reject(F f){
    try{f();}catch(const E&){return;}throw std::runtime_error("expected projection rejection");
}
void hand_witness(){
    const auto m=perspective_matrix({},DepthMap{1,9,DepthOrder::Forward});
    const auto t=trace_projection(m,{}, {1,1,-4});
    require(t.classification.inside()&&t.screen.has_value(),"front point inside");
    near(t.clip.x,1.25);near(t.clip.y,5.0/3);near(t.clip.z,3.375);near(t.clip.w,4);
    near(t.ndc->x,.3125);near(t.ndc->y,5.0/12);near(t.ndc->z,.84375);
    near(t.screen->image.u,420);near(t.screen->image.v,140);
    const auto r=trace_projection(perspective_matrix({}, {1,9,DepthOrder::Reverse}),{}, {1,1,-4});
    near(r.screen->depth,.15625);near(r.screen->image.u,420);
    require(!trace_projection(m,{}, {0,0,1}).screen,"behind camera rejected");
    require(!trace_projection(m,{}, {0,0,-.5}).screen,"near rejection");
    require(!trace_projection(m,{}, {0,0,-10}).screen,"far rejection");
    require(!trace_projection(m,{}, {10,0,-4}).screen,"side rejection");
    require(!trace_projection(m,{}, {0,0,0}).screen,"zero w rejected");
    // Exact clip boundaries: closed geometric region, not a pixel index.
    require(classify_clip({-2,2,0,2}).inside(),"clip boundary included");
    require(!classify_clip({0,0,0,0}).inside(),"homogeneous zero rejected");
    require(!classify_clip({0,0,-.001,1}).inside(),"negative clip z");
    const auto v=Viewport{10,20,800,600}.map({1,-1,.5});
    near(v.image.u,810);near(v.image.v,620);near(v.depth,.5);
}
void depth_contract(){
    for(auto order:{DepthOrder::Forward,DepthOrder::Reverse}){
        const bool rev=order==DepthOrder::Reverse;
        for(std::optional<double> far:{std::optional<double>{1000},std::nullopt}){
            DepthMap d{.5,far,order};
            near(d.perspective_depth(.5),rev?1:0);
            if(far)near(d.perspective_depth(*far),rev?0:1);
            const auto endpoint=d.inverse_perspective(rev?0:1);
            require(endpoint==far,"finite or infinite far endpoint");
            for(double z:{.5,1.,2.,10.,100.,999.}){
                const auto q=d.perspective_depth(z);
                near(*d.inverse_perspective(q),z,1e-8);
            }
        }
    }
    DepthMap infinite{.1,std::nullopt,DepthOrder::Reverse};
    near(infinite.perspective_depth(1000),.0001);
    near(*infinite.inverse_perspective(.0001),1000);
    require(!infinite.inverse_perspective(0),"infinite endpoint");
    reject<std::invalid_argument>([]{(void)DepthMap{1,1}.perspective_coefficients();});
    reject<std::invalid_argument>([]{(void)DepthMap{0,100}.perspective_coefficients();});
    reject<std::invalid_argument>([&]{(void)infinite.inverse_perspective(-1);});
}
void orthographic(){
    const OrthoBounds b{-2,6,-1,3};
    auto m=orthographic_matrix(b,{1,9,DepthOrder::Forward});
    auto a=trace_projection(m,{}, {2,1,-3});
    near(a.clip.w,1);near(a.ndc->x,0);near(a.ndc->y,0);near(a.ndc->z,.25);
    auto c=trace_projection(m,{}, {2,1,-7});
    near(c.ndc->x,0);near(c.ndc->y,0);near(c.ndc->z,.75);
    auto r=trace_projection(orthographic_matrix(b,{1,9,DepthOrder::Reverse}),{}, {2,1,-3});
    near(r.ndc->z,.75);
    reject<std::invalid_argument>([]{(void)orthographic_matrix({}, {1,std::nullopt});});
}
void calibrated_crosscheck(){
    CameraIntrinsics k{800,600,500,450,410,290,12};
    PinholeCamera c{k,{}};
    DepthMap depth{.1,100,DepthOrder::Reverse};
    auto m=perspective_matrix(k,depth);
    std::mt19937_64 rng(140014);
    auto s=[&]{return static_cast<double>(rng()>>11)*0x1.0p-53;};
    for(int i=0;i<4000;++i){
        Point3 p{4*s()-2,4*s()-2,-(5+40*s())};
        auto pin=c.project(p);
        auto t=trace_projection(m,{0,0,800,600},p);
        require(t.classification.inside()&&t.screen.has_value(),"seeded interior point");
        near(t.screen->image.u,pin.image.u);
        near(t.screen->image.v,pin.image.v);
        near(t.screen->depth,depth.perspective_depth(-p.z));
        near(*depth.inverse_perspective(t.screen->depth),-p.z);
    }
}
}
int main(){try{hand_witness();depth_contract();orthographic();calibrated_crosscheck();
    std::cout<<"Book03/P14 projection and depth contracts: PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
