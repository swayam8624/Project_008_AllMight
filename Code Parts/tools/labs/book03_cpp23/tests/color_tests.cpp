#include "book03/color.hpp"
#include <iostream>
#include <random>
using namespace book03::color;
void check(bool ok) {if(!ok)throw std::runtime_error("color contract");}
int main(){
    const auto w=xyY(mul(srgb,{1,1,1})).value();
    check(std::abs(w[0]-.3127)<1e-12&&std::abs(w[1]-.3290)<1e-12&&std::abs(w[2]-1)<1e-12);
    check(!xyY({0,0,0}));
    check(std::abs(encode(.5)-.7353569830524495)<1e-14);
    check(std::abs(decode(.5)-.21404114048223255)<1e-14);
    const auto r=convert({1,0,0},p3,srgb);check(r[0]>1&&r[1]<0&&r[2]<0&&!in_gamut(r));
    const auto neutral=apply_gains({.4,.6,.3},neutral_gains({.4,.6,.3}));
    for(double x:neutral)check(std::abs(x-.6)<1e-14);
    const auto h=compress({8,4,2},1);check(std::abs(h[0]/h[1]-2)<1e-14&&std::abs(h[1]/h[2]-2)<1e-14);
    std::mt19937_64 rng(25);std::uniform_real_distribution<double> u(0,1);double e=0,t=0;
    for(int k=0;k<10000;++k){Triple v{u(rng),u(rng),u(rng)};auto a=convert(convert(v,srgb,p3),p3,srgb);for(int i=0;i<3;++i){e=std::max(e,std::abs(a[i]-v[i]));t=std::max(t,std::abs(decode(std::clamp(encode(v[i]),0.,1.))-v[i]));}}
    check(e<5e-14&&t<5e-8);
    for(int k=0;k<256;++k)check(std::abs(encode(decode(k/255.))-k/255.)<1e-12);
    try{(void)decode(-.1);check(false);}catch(const std::invalid_argument&){}
    try{(void)neutral_gains({0,1,1});check(false);}catch(const std::invalid_argument&){}
    std::cout<<"P25 PASS matrix max="<<e<<" transfer max="<<t<<'\n';
}
