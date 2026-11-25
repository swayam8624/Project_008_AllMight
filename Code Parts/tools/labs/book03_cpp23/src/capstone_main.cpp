#include "book03/capstone.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
int main(int argc,char** argv){
    if(argc!=2)return 2;
    std::ofstream out(argv[1]);if(!out)return 1;
    out<<std::setprecision(17)<<"trial,baseline_m,sigma_px,attempted,accepted,depth_rmse_m,position_rmse_m,pixel_rmse_px,predicted_depth_sigma_m\n";
    std::mt19937_64 rng(3201);std::uniform_real_distribution<double> u(-1,1);std::normal_distribution<double> n;
    for(int trial=0;trial<20;++trial){std::vector<book03::capstone::Sample> samples;for(int i=0;i<200;++i){book03::capstone::Sample s{{u(rng),u(rng),4},{}};for(auto& z:s.noise)z=n(rng);samples.push_back(s);}
        for(double b:{.05,.1,.2,.4,.8}){auto r=book03::capstone::evaluate(samples,b,.25);out<<trial<<','<<b<<",0.25,"<<r.attempted<<','<<r.accepted<<','<<r.depth_rmse<<','<<r.position_rmse<<','<<r.pixel_rmse<<','<<book03::capstone::predicted_depth_sigma(4,b,.25)<<'\n';}}
    return out?0:1;
}
