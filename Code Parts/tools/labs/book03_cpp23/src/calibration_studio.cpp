#include "book03/calibration.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
using namespace book03::calibration;
int main(int argc,char** argv){if(argc!=2)return 2;std::ofstream out(argv[1]);if(!out)return 1;out<<std::setprecision(17)<<"sigma,trial,train_rms,heldout_rms,fx,fy,cx,cy,skew,gap_ratio\n";
    const Camera p{{{800,4,320,1280},{0,820,240,960},{0,0,1,4}}};std::mt19937_64 rng(2601);std::normal_distribution<double> normal;
    for(double sigma:{0.,.1,.25,.5,1.})for(int trial=0;trial<20;++trial){std::vector<Match> obs;for(int x=-2;x<=2;++x)for(int y=-2;y<=2;++y)for(int z=0;z<3;++z){V3 w{.3*x,.3*y,.3*z};auto u=project(p,w);for(double& v:u)v+=sigma*normal(rng);obs.push_back({w,u});}auto fit=dlt(obs);auto k=decompose(fit.p);double e=0;for(int i=0;i<20;++i){V3 w{-.53+.05*i,.23,.17+.02*i};auto a=project(fit.p,w),b=project(p,w);e+=(std::pow(a[0]-b[0],2)+std::pow(a[1]-b[1],2))/20;}out<<sigma<<','<<trial<<','<<fit.rms_pixels<<','<<std::sqrt(e)<<','<<k.fx<<','<<k.fy<<','<<k.cx<<','<<k.cy<<','<<k.skew<<','<<fit.gap_ratio<<'\n';}
    return out?0:1;
}
