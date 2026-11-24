#include "book03/epipolar.hpp"
#include <iostream>
#include <random>
using namespace book03::epipolar;
void check(bool ok){if(!ok)throw std::runtime_error("epipolar contract");}
V pixel(V x){return {800*x[0]/x[2]+320,820*x[1]/x[2]+240,1};}
int main(){
    const double c=std::cos(.13),s=std::sin(.13);M r{{{c,0,s},{0,1,0},{-s,0,c}}};V t{-.3,.04,.02};
    auto ki=inverse_k(800,820,0,320,240);auto f=fundamental(essential(r,t),ki,ki);
    std::mt19937_64 rng(27);std::uniform_real_distribution<double> u(-1,1);std::vector<Match> obs;
    for(int i=0;i<100;++i){V x{u(rng),u(rng),4+u(rng)},y=mul(r,x);for(int j=0;j<3;++j)y[j]+=t[j];Match m{pixel(x),pixel(y)};check(sampson(f,m)<1e-20);obs.push_back(m);}
    auto fit=eight_point(obs);double worst=0;for(auto m:obs)worst=std::max(worst,sampson(fit.f,m));check(worst<1e-16);
    auto expected=normalized(f);double plus=0,minus=0;for(int i=0;i<3;++i)for(int j=0;j<3;++j){plus+=std::pow(expected[i][j]+fit.f[i][j],2);minus+=std::pow(expected[i][j]-fit.f[i][j],2);}check(std::min(plus,minus)<1e-16);
    std::vector<Match> flat;for(int i=0;i<100;++i){V x{u(rng),u(rng),4},y=mul(r,x);for(int j=0;j<3;++j)y[j]+=t[j];flat.push_back({pixel(x),pixel(y)});}try{(void)eight_point(flat);check(false);}catch(const std::domain_error&){}
    auto scaled=f;for(auto& row:scaled)for(double& z:row)z*=7;auto bad=obs[0];bad.b[1]+=4;check(std::abs(sampson(f,bad)-sampson(scaled,bad))<1e-10);check(sampson(f,bad)>1);
    std::cout<<"P27 PASS max exact Sampson squared="<<worst<<'\n';
}
