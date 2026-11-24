#include "book03/sfm.hpp"
#include <fstream>
#include <iomanip>
#include <random>
using namespace book03::sfm;
int main(int argc,char** argv){if(argc!=2)return 2;std::ofstream out(argv[1]);if(!out)return 1;out<<std::setprecision(17)<<"sigma,trial,initial_rms,refined_rms,initial_3d_rms,refined_3d_rms\n";M k{{{800,0,320},{0,820,240},{0,0,1}}};auto ki=book03::epipolar::inverse_k(800,820,0,320,240);std::vector<Pose> poses{Pose{},Pose{identity,{-1,0,0}},Pose{identity,{.2,-.5,.1}}};std::mt19937_64 rng(2901);std::uniform_real_distribution<double> u(-1,1);std::normal_distribution<double> n;
for(double sigma:{0.,.1,.25,.5,1.})for(int trial=0;trial<20;++trial){double a=0,b=0,c=0,d=0;for(int i=0;i<100;++i){V x{u(rng),u(rng),8+2*u(rng)};Track tr;for(std::size_t j=0;j<poses.size();++j){auto p=project(poses[j],k,x);for(int q=0;q<2;++q)p[q]+=sigma*n(rng);tr.push_back({j,p});}auto y=triangulate(poses,ki,tr),z=refine_point(poses,k,tr,y);a+=cost(poses,k,tr,y)/300;b+=cost(poses,k,tr,z)/300;c+=std::pow(norm(add(y,x,-1)),2)/100;d+=std::pow(norm(add(z,x,-1)),2)/100;}out<<sigma<<','<<trial<<','<<std::sqrt(a)<<','<<std::sqrt(b)<<','<<std::sqrt(c)<<','<<std::sqrt(d)<<'\n';}
return out?0:1;}
