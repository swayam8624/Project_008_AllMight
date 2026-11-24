#include "book03/sampling.hpp"
#include <iostream>
#include <random>
using namespace book03;
void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{
    Wave low{.2,.3},high{1.2,.3-std::numbers::pi};
    for(int i=0;i<64;++i)check(std::abs(low.at(i+.5)-high.at(i+.5))<1e-12,"alias witness");
    for(int i=0;i<64;++i)check(std::abs(Wave{1,0}.box(i+.5)-.5)<1e-14,"integer cycle integral");
    check(std::abs(Wave{0,0}.box(.5)-1)<1e-14,"DC");
    check(std::abs(srgb_encode(.5)-.7353569830524495)<1e-14,"linear resolve");
    std::mt19937 rng(200020);
    for(int i=0;i<2000;++i){
        double f=(rng()%4001)/1000.,phase=(rng()%6283)/1000.;
        double center=(rng()%64)+.5;
        Wave s{f,phase};
        // Composite midpoint error <= max |s''| / (24*n^2).
        constexpr int n=256;
        double bound=.5*std::pow(2*std::numbers::pi*f,2)/(24*n*n);
        check(std::abs(midpoint_box(s,center,n)-s.box(center))<=bound+1e-12,"quadrature bound");
        double edge=center+(int(rng()%2001)-1000)/1000.;
        check(std::abs(step_samples(edge,center,16)-step_box(edge,center))<=1./32+1e-12,"step coverage bound");
    }
    std::cout<<"Book03/P20 sampling contracts: PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
