#pragma once
// P20: scalar, linear-light sampling of an analytic continuous signal.
// Pixel footprint [i,i+1]; phase in radians; frequency in cycles/pixel.
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
namespace book03 {
struct Wave {
    double frequency{},phase{};
    void validate() const {
        if(!std::isfinite(frequency)||frequency<0||frequency>64||
           !std::isfinite(phase)||std::abs(phase)>100)
            throw std::invalid_argument("wave outside teaching domain");
    }
    double at(double x) const {
        validate();
        if(!std::isfinite(x)||std::abs(x)>4096)throw std::invalid_argument("sample coordinate");
        return .5+.5*std::cos(2*std::numbers::pi*frequency*x+phase);
    }
    // Exact continuous box integral in real arithmetic; stable near DC.
    double box(double center) const {
        const double c=at(center),z=std::numbers::pi*frequency;
        const double sinc=z==0?1:std::sin(z)/z;
        return .5+(c-.5)*sinc;
    }
};
inline double midpoint_box(const Wave& s,double center,int n){
    if(n<1||n>1024)throw std::invalid_argument("sample count");
    double sum=0;
    for(int k=0;k<n;++k)sum+=s.at(center-.5+(k+.5)/n);
    return sum/n;
}
// Coverage of a vertical step across a unit pixel: fraction to its right.
inline double step_box(double edge,double center){
    if(!std::isfinite(edge)||!std::isfinite(center))throw std::invalid_argument("step coordinate");
    return std::clamp(center+.5-edge,0.,1.);
}
inline double step_samples(double edge,double center,int n){
    if(n<1||n>1024||!std::isfinite(edge)||!std::isfinite(center))
        throw std::invalid_argument("step sampling domain");
    int hits=0;
    for(int k=0;k<n;++k)hits+=center-.5+(k+.5)/n>=edge;
    return double(hits)/n;
}
inline double srgb_encode(double x){
    if(!std::isfinite(x)||x<0||x>1)throw std::invalid_argument("linear value");
    return x<=.0031308?12.92*x:1.055*std::pow(x,1./2.4)-.055;
}
} // namespace book03
