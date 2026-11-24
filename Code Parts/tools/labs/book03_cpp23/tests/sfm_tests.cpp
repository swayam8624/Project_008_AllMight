#include "book03/sfm.hpp"
#include <iostream>
#include <random>
using namespace book03::sfm;
void check(bool v){if(!v)throw std::runtime_error("sfm contract");}
int main(){M k{{{800,0,320},{0,820,240},{0,0,1}}};auto ki=book03::epipolar::inverse_k(800,820,0,320,240);double c=std::cos(.13),s=std::sin(.13);Pose p{{{{c,0,s},{0,1,0},{-s,0,c}}},unit({-.3,.04,.02})};std::vector<Pose> poses{Pose{},p,Pose{identity,{.2,-.5,.1}}};std::mt19937_64 rng(29);std::uniform_real_distribution<double> u(-1,1);std::normal_distribution<double> n(0,.3);std::vector<book03::epipolar::Match> matches;std::vector<V> points;
for(int i=0;i<80;++i){V x{u(rng),u(rng),8+u(rng)*2};points.push_back(x);matches.push_back({project(poses[0],k,x),project(p,k,x)});}auto f=book03::epipolar::eight_point(matches);auto seed=recover(f.f,k,ki,matches);check(norm(add(seed.pose.t,p.t,-1))<1e-8);for(int i=0;i<3;++i)check(norm(add(seed.pose.r[i],p.r[i],-1))<1e-8);
double worst=0;int improved=0;for(auto x:points){Track tr;for(std::size_t j=0;j<poses.size();++j)tr.push_back({j,project(poses[j],k,x)});auto a=triangulate(poses,ki,tr);worst=std::max(worst,norm(add(a,x,-1)));for(auto& o:tr)for(int j=0;j<2;++j)o.pixel[j]+=n(rng);a=triangulate(poses,ki,tr);auto b=refine_point(poses,k,tr,a);check(cost(poses,k,tr,b)<=cost(poses,k,tr,a)+1e-10);improved+=cost(poses,k,tr,b)<cost(poses,k,tr,a)-1e-12;}
check(worst<1e-9&&improved>70);try{(void)triangulate({Pose{},Pose{}},ki,{{0,{320,240,1}},{1,{320,240,1}}});check(false);}catch(const std::domain_error&){}try{(void)triangulate(poses,ki,{{0,{320,240,1}},{0,{320,240,1}}});check(false);}catch(const std::invalid_argument&){}std::cout<<"P29 PASS exact 3D max="<<worst<<" improved="<<improved<<" votes=";for(auto v:seed.votes)std::cout<<v<<',';std::cout<<'\n';}
