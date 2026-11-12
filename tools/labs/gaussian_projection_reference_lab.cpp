// PROJECTALLMIGHT Phase 25 witness: 3D Gaussian covariance projection, ellipse eigenvalues, SH basis, compositing.
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }
bool near(double a,double b,double e=1e-9){ return std::fabs(a-b)<=e; }

using M3=std::array<std::array<double,3>,3>;
using J2=std::array<std::array<double,3>,2>;
using M2=std::array<std::array<double,2>,2>;

M2 project_covariance(const M3& s,const J2& j){
    M2 out{};
    for(int r=0;r<2;++r) for(int c=0;c<2;++c)
        for(int i=0;i<3;++i) for(int k=0;k<3;++k)
            out[r][c]+=j[r][i]*s[i][k]*j[c][k];
    return out;
}

J2 perspective_jacobian(double x,double y,double z,double fx,double fy){
    require(z>0,"camera-space z must be positive");
    return {{{fx/z,0.0,-fx*x/(z*z)},
             {0.0,fy/z,-fy*y/(z*z)}}};
}

std::pair<double,double> eigenvalues_sym2(const M2& a){
    require(std::fabs(a[0][1]-a[1][0])<1e-9,"matrix must be symmetric");
    const double tr=a[0][0]+a[1][1];
    const double disc=std::sqrt(std::max(0.0,(a[0][0]-a[1][1])*(a[0][0]-a[1][1])+4*a[0][1]*a[0][1]));
    return {(tr+disc)*0.5,(tr-disc)*0.5};
}

std::array<double,2> project(double x,double y,double z,double fx,double fy){
    require(z>0,"z must be positive");
    return {fx*x/z,fy*y/z};
}

void test_jacobian_by_finite_difference(){
    const double x=0.4,y=-0.2,z=2.5,fx=900,fy=850;
    const J2 j=perspective_jacobian(x,y,z,fx,fy);
    const double h=1e-6;
    const auto base=project(x,y,z,fx,fy);
    for(int axis=0;axis<3;++axis){
        double xx=x,yy=y,zz=z;
        if(axis==0) xx+=h;
        if(axis==1) yy+=h;
        if(axis==2) zz+=h;
        const auto p=project(xx,yy,zz,fx,fy);
        require(std::fabs((p[0]-base[0])/h-j[0][axis])<3e-3,"finite-difference mismatch in projection Jacobian x row");
        require(std::fabs((p[1]-base[1])/h-j[1][axis])<3e-3,"finite-difference mismatch in projection Jacobian y row");
    }
}

void test_covariance_psd_and_scale(){
    const M3 sigma{{{0.04,0.01,0.0},{0.01,0.09,0.0},{0.0,0.0,0.01}}};
    const J2 j=perspective_jacobian(0.3,-0.1,3.0,1000,1000);
    const M2 c=project_covariance(sigma,j);
    require(near(c[0][1],c[1][0],1e-9),"projected covariance lost symmetry");
    const auto [lmax,lmin]=eigenvalues_sym2(c);
    require(lmin>=-1e-9 && lmax>=lmin,"projected covariance must be positive semidefinite");
    require(lmax>0,"projected footprint should be nonzero");

    // Doubling focal length doubles J and therefore multiplies covariance eigenvalues by four.
    const M2 c2=project_covariance(sigma,perspective_jacobian(0.3,-0.1,3.0,2000,2000));
    const auto [lmax2,lmin2]=eigenvalues_sym2(c2);
    require(std::fabs(lmax2/lmax-4.0)<1e-9 && std::fabs(lmin2/lmin-4.0)<1e-9,
            "focal-length covariance scaling mismatch");
}

std::array<double,4> sh_l1(double x,double y,double z){
    // Real SH basis through l=1 under one common graphics convention.
    return {0.28209479177387814,
            -0.4886025119029199*y,
             0.4886025119029199*z,
            -0.4886025119029199*x};
}

void test_sh_directionality(){
    const auto z=sh_l1(0,0,1);
    const auto minus_z=sh_l1(0,0,-1);
    require(near(z[0],minus_z[0]),"l=0 basis must be direction independent");
    require(near(z[2],-minus_z[2]),"l=1 z basis must change sign with direction");
}

struct Layer{ double alpha; std::array<double,3> color; };

std::pair<std::array<double,3>,double> composite_front_to_back(const std::vector<Layer>& layers){
    std::array<double,3> c{0,0,0};
    double transmittance=1.0;
    for(const Layer& l:layers){
        require(l.alpha>=0&&l.alpha<=1,"alpha outside [0,1]");
        const double weight=transmittance*l.alpha;
        for(int k=0;k<3;++k) c[k]+=weight*l.color[k];
        transmittance*=1.0-l.alpha;
    }
    return {c,1.0-transmittance};
}

void test_compositing_order_and_opacity(){
    const Layer red{0.5,{1,0,0}};
    const Layer blue{0.5,{0,0,1}};
    auto [rb,arb]=composite_front_to_back({red,blue});
    auto [br,abr]=composite_front_to_back({blue,red});
    require(near(arb,0.75)&&near(abr,0.75),"combined opacity must be 1-product(1-alpha)");
    require(rb!=br,"color compositing should expose front-to-back order dependence");
    require(near(rb[0],0.5)&&near(rb[2],0.25),"red-then-blue compositing trace mismatch");
}

} // namespace

int main(){
    try{
        test_jacobian_by_finite_difference();
        test_covariance_psd_and_scale();
        test_sh_directionality();
        test_compositing_order_and_opacity();
        std::cout<<"gaussian_projection_reference_lab: PASS\n"
                 <<"proved: perspective Jacobian finite-difference agreement, J*Sigma*J^T symmetry/PSD, focal scaling, l<=1 SH directionality, fixed-order alpha compositing\n"
                 <<"not proved: full 3D covariance parameterization, conic inversion, tile duplication/sort, higher-order SH, GPU rasterization, temporal filtering, or Maveb/Vulkax parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"gaussian_projection_reference_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
