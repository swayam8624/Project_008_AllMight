#include "book03/sensor.hpp"
#include <iostream>
using namespace book03::sensor;
void check(bool ok) { if(!ok) throw std::runtime_error("sensor contract failed"); }
int main() {
    check(std::abs(photons(1,1e-12,0.01,550e-9)-27687.6411)<0.01);
    Config p; p.qe=0.5; p.dark_per_second=2; p.read_sigma=3;
    const auto m=moments(p,200,1);
    check(m.signal==100 && m.dark==2 && m.variance==111);
    check(adc(p,-10000)==0 && adc(p,1e8)==4095 && adc(p,0)==64);
    std::mt19937_64 rng(24); constexpr int n=100000;
    double sum=0, sq=0;
    for(int i=0;i<n;++i) { const auto a=capture(p,200,1,rng); sum+=a.analog; sq+=a.analog*a.analog; }
    const double mean=sum/n, var=sq/n-mean*mean;
    check(std::abs(mean-102)<0.2 && std::abs(var-111)<3);
    p.full_well=10; p.read_sigma=0;
    for(int i=0;i<100;++i) {const auto a=capture(p,2000,1,rng); check(a.stored==10 && a.analog==10);}
    p.qe=0; p.dark_per_second=0; check(capture(p,2000,1,rng).charge==0);
    p.bits=0; try { (void)adc(p,1); check(false); } catch(const std::invalid_argument&) {}
    std::cout<<"P24 PASS mean="<<mean<<" variance="<<var<<'\n';
}
