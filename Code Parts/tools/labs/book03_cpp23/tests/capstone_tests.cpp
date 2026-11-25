#include "book03/capstone.hpp"
#include <iostream>
#include <sstream>
using namespace book03;
void check(bool b){if(!b)throw std::runtime_error("capstone contract");}
int main(){
    auto scene=studio::house();auto poses=studio::cameras();auto ki=epipolar::inverse_k(160,160,0,160,120);auto original=studio::render(scene);
    double max_scale_error=0,max_jac_error=0;
    for(double factor:{.5,2.}){auto scaled=scene;auto assumed=poses;for(auto& p:assumed)p.t=sfm::scale(p.t,factor);
        for(std::size_t i=0;i<scene.points.size();++i){auto p=scene.points[i];sfm::Track tr;for(std::size_t j=0;j<poses.size();++j)tr.push_back({j,sfm::project(poses[j],studio::k,p)});auto x=sfm::triangulate(assumed,ki,tr);max_scale_error=std::max(max_scale_error,sfm::norm(sfm::add(x,sfm::scale(p,factor),-1)));check(sfm::cost(assumed,studio::k,tr,x)<1e-18);scaled.points[i]=x;}
        auto image=studio::render(scaled);std::ostringstream a,b;original.ppm(a);image.ppm(b);check(a.str()==b.str());
    }
    check(max_scale_error<1e-10);
    // Independently difference the projection at multiple noncentral positions.
    for(auto p:scene.points){auto j=sfm::jacobian(studio::k,p);for(int axis=0;axis<3;++axis){auto a=p,b=p;a[axis]+=1e-5;b[axis]-=1e-5;auto u=sfm::project(sfm::Pose{},studio::k,a),v=sfm::project(sfm::Pose{},studio::k,b);for(int c=0;c<2;++c)max_jac_error=std::max(max_jac_error,std::abs((u[c]-v[c])/2e-5-j[c][axis]));}}
    check(max_jac_error<1e-7);
    try{(void)sfm::triangulate({sfm::Pose{},sfm::Pose{}},ki,{{0,{160,120,1}},{1,{160,120,1}}});check(false);}catch(const std::domain_error&){}
    std::vector<capstone::Sample> samples{{{.2,-.3,4},{0,0,0,0}},{{-.3,.1,4},{0,0,0,0}}};for(double b:{.05,.1,.2,.4,.8}){auto r=capstone::evaluate(samples,b,0);check(r.accepted==2&&r.position_rmse<1e-9);}
    std::cout<<"P32 PASS scale_error="<<max_scale_error<<" jacobian_error="<<max_jac_error<<'\n';
}
