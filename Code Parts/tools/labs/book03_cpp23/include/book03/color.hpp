#pragma once
// P25: relative D65 colorimetry. Linear RGB is not encoded RGB.
#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <stdexcept>
namespace book03::color {
using Triple=std::array<double,3>;
using Matrix=std::array<Triple,3>;
inline void finite(Triple v) { for(double x:v) if(!std::isfinite(x)) throw std::invalid_argument("nonfinite color"); }
inline Triple mul(const Matrix& m,Triple v) {
    finite(v); Triple r{};
    for(int i=0;i<3;++i) for(int j=0;j<3;++j) r[i]+=m[i][j]*v[j];
    finite(r); return r;
}
// D65, Y_white=1. Rows are X,Y,Z; columns are R,G,B.
inline constexpr Matrix srgb{{{.4123907992659595,.3575843393838780,.1804807884018343},{.2126390058715104,.7151686787677560,.0721923153607337},{.0193308187155919,.1191947797946260,.9505321522496606}}};
inline constexpr Matrix p3{{{.4865709486482162,.2656676931690930,.1982172852343625},{.2289745640697488,.6917385218365064,.0792869140937450},{0,.0451133818589026,1.0439443689009760}}};
inline Matrix inverse(const Matrix& m) {
    for(auto row:m) finite(row);
    Matrix a{};
    for(int i=0;i<3;++i) for(int j=0;j<3;++j) {
        int r=(j+1)%3,s=(j+2)%3,u=(i+1)%3,v=(i+2)%3;
        a[i][j]=m[r][u]*m[s][v]-m[r][v]*m[s][u];
    }
    const double d=m[0][0]*a[0][0]+m[0][1]*a[1][0]+m[0][2]*a[2][0];
    // Intended for the two fixed, well-scaled matrices above, not a general solver.
    if(!std::isfinite(d)||std::abs(d)<1e-12) throw std::invalid_argument("singular or badly scaled matrix");
    for(auto& row:a) {for(auto& x:row)x/=d;finite(row);} return a;
}
inline Triple convert(Triple linear,const Matrix& from,const Matrix& to) {return mul(inverse(to),mul(from,linear));}
inline double unit(double x) {if(!std::isfinite(x)||x<0||x>1)throw std::invalid_argument("unit interval required");return x;}
inline double encode(double x) {unit(x);return x<=.0031308?12.92*x:1.055*std::pow(x,1./2.4)-.055;}
inline double decode(double x) {unit(x);return x<=.04045?x/12.92:std::pow((x+.055)/1.055,2.4);}
// A signed RGB result is useful evidence: do not silently clip in convert().
inline bool in_gamut(Triple v,double tolerance=1e-12) {finite(v);if(!std::isfinite(tolerance)||tolerance<0||tolerance>1e-6)throw std::invalid_argument("gamut tolerance");for(double x:v)if(x< -tolerance||x>1+tolerance)return false;return true;}
inline Triple clip(Triple v) {finite(v);for(auto& x:v)x=std::clamp(x,0.,1.);return v;}
inline std::optional<Triple> xyY(Triple xyz) {
    finite(xyz);for(double x:xyz)if(x<0)throw std::invalid_argument("negative XYZ");
    const double s=xyz[0]+xyz[1]+xyz[2];
    if(!std::isfinite(s))throw std::overflow_error("XYZ sum");
    if(s==0)return std::nullopt;
    return Triple{xyz[0]/s,xyz[1]/s,xyz[1]};
}
// Simplified RAW-domain neutral correction; G is held fixed.
inline Triple neutral_gains(Triple measured) {
    finite(measured);for(double x:measured)if(x<=0)throw std::invalid_argument("neutral must be positive");
    Triple g{measured[1]/measured[0],1,measured[1]/measured[2]};finite(g);return g;
}
inline Triple apply_gains(Triple raw,Triple gains) {finite(raw);finite(gains);for(int i=0;i<3;++i){if(gains[i]<=0)throw std::invalid_argument("positive gain required");raw[i]*=gains[i];}finite(raw);return raw;}
// Simple chromaticity-preserving relative HDR compression for nonnegative linear RGB.
inline Triple compress(Triple rgb,double exposure) {
    finite(rgb);for(double x:rgb)if(x<0)throw std::invalid_argument("negative HDR RGB");
    if(!std::isfinite(exposure)||exposure<0)throw std::invalid_argument("exposure");
    for(auto& x:rgb)x*=exposure;
    const double y=mul(srgb,rgb)[1];
    for(auto& x:rgb)x/=1+y;
    finite(rgb);return rgb; // Not guaranteed in gamut; explicit display policy follows.
}
} // namespace book03::color
