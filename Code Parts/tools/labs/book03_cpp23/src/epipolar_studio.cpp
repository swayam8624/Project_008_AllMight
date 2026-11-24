#include "book03/epipolar.hpp"
#include <fstream>
#include <iomanip>
#include <random>
using namespace book03::epipolar;
V pixel(V x){return {800*x[0]/x[2]+320,820*x[1]/x[2]+240,1};}
int main(int argc,char** argv){
    if(argc!=2)return 2;
    std::ofstream out(argv[1]);if(!out)return 1;
    out<<std::setprecision(17)<<"sigma,trial,train_sampson_rms,heldout_sampson_rms,gap\n";
    double c=std::cos(.13),s=std::sin(.13);M r{{{c,0,s},{0,1,0},{-s,0,c}}};V t{-.3,.04,.02};
    std::mt19937_64 rng(2701);std::uniform_real_distribution<double> u(-1,1);std::normal_distribution<double> normal;
    auto sample=[&](){V x{u(rng),u(rng),4+u(rng)},y=mul(r,x);for(int j=0;j<3;++j)y[j]+=t[j];return Match{pixel(x),pixel(y)};};
    for(double sigma:{0.,.1,.25,.5,1.})for(int trial=0;trial<20;++trial){
        std::vector<Match> obs;for(int i=0;i<100;++i){auto m=sample();for(int j=0;j<2;++j){m.a[j]+=sigma*normal(rng);m.b[j]+=sigma*normal(rng);}obs.push_back(m);}auto fit=eight_point(obs);double e=0,h=0;for(auto m:obs)e+=sampson(fit.f,m)/100;for(int i=0;i<50;++i)h+=sampson(fit.f,sample())/50;
        out<<sigma<<','<<trial<<','<<std::sqrt(e)<<','<<std::sqrt(h)<<','<<fit.gap<<'\n';
    }return out?0:1;
}
