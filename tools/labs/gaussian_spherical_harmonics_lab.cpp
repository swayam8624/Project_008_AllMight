// PROJECTALLMIGHT real spherical-harmonics l<=3 oracle for Gaussian appearance.
// Convention is stated explicitly; signs can differ in other SH conventions without changing orthonormality.
#include <array>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <string>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

struct V3{ double x{},y{},z{}; };

std::array<double,16> sh16(V3 d){
    const double r=std::sqrt(d.x*d.x+d.y*d.y+d.z*d.z);
    require(r>0,"SH direction must be nonzero");
    const double x=d.x/r,y=d.y/r,z=d.z/r;

    std::array<double,16> b{};
    b[0]  = 0.28209479177387814;
    b[1]  = 0.4886025119029199*y;
    b[2]  = 0.4886025119029199*z;
    b[3]  = 0.4886025119029199*x;
    b[4]  = 1.0925484305920792*x*y;
    b[5]  = 1.0925484305920792*y*z;
    b[6]  = 0.31539156525252005*(3.0*z*z-1.0);
    b[7]  = 1.0925484305920792*x*z;
    b[8]  = 0.5462742152960396*(x*x-y*y);
    b[9]  = 0.5900435899266435*y*(3.0*x*x-y*y);
    b[10] = 2.890611442640554*x*y*z;
    b[11] = 0.4570457994644658*y*(5.0*z*z-1.0);
    b[12] = 0.3731763325901154*z*(5.0*z*z-3.0);
    b[13] = 0.4570457994644658*x*(5.0*z*z-1.0);
    b[14] = 1.445305721320277*z*(x*x-y*y);
    b[15] = 0.5900435899266435*x*(x*x-3.0*y*y);
    return b;
}

V3 fibonacci_direction(std::size_t i,std::size_t n){
    const double golden=std::numbers::pi*(3.0-std::sqrt(5.0));
    const double z=1.0-2.0*(static_cast<double>(i)+0.5)/static_cast<double>(n);
    const double radius=std::sqrt(std::max(0.0,1.0-z*z));
    const double phi=golden*static_cast<double>(i);
    return {radius*std::cos(phi),radius*std::sin(phi),z};
}

void test_basis_values(){
    const auto north=sh16({0,0,1});
    require(std::fabs(north[1])<1e-15&&std::fabs(north[3])<1e-15,
            "l=1 tangent terms must vanish at north pole");
    require(std::fabs(north[2]-0.4886025119029199)<1e-14,
            "Y10 north-pole value changed");
    require(std::fabs(north[6]-0.6307831305050401)<1e-13,
            "Y20 north-pole value changed");
}

void test_orthonormality(){
    constexpr std::size_t n=60000;
    std::array<std::array<double,16>,16> gram{};
    const double weight=4.0*std::numbers::pi/static_cast<double>(n);
    for(std::size_t i=0;i<n;++i){
        const auto y=sh16(fibonacci_direction(i,n));
        for(std::size_t a=0;a<16;++a)
            for(std::size_t b=0;b<16;++b)
                gram[a][b]+=y[a]*y[b]*weight;
    }
    double worst_diag=0,worst_cross=0;
    for(std::size_t a=0;a<16;++a){
        worst_diag=std::max(worst_diag,std::fabs(gram[a][a]-1.0));
        for(std::size_t b=0;b<16;++b){
            if(a!=b) worst_cross=std::max(worst_cross,std::fabs(gram[a][b]));
        }
    }
    require(worst_diag<0.0025,"SH diagonal normalization error too large");
    require(worst_cross<0.0025,"SH basis cross-correlation too large");
}

void test_constant_reconstruction(){
    constexpr double color=0.73;
    const double y00=sh16({1,2,3})[0];
    const double coefficient=color/y00;
    for(V3 d: {V3{1,0,0},V3{0,1,0},V3{0,0,1},V3{-2,1,4}}){
        const double reconstructed=coefficient*sh16(d)[0];
        require(std::fabs(reconstructed-color)<1e-14,
                "l=0 coefficient failed to reconstruct direction-independent value");
    }
}

} // namespace

int main(){
    try{
        test_basis_values();
        test_orthonormality();
        test_constant_reconstruction();
        std::cout
            <<"gaussian_spherical_harmonics_lab: PASS\n"
            <<"proved: explicit real SH convention through degree 3, pole values, numerical spherical orthonormality of all 16 basis functions, l=0 constant reconstruction\n"
            <<"not proved: 3DGS-specific sign/channel convention parity, trained coefficient ranges, view-direction convention, GPU evaluation, clamping/color-space policy, or Maveb/Vulkax parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"gaussian_spherical_harmonics_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
