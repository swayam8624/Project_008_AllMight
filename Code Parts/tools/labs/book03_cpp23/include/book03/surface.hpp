#pragma once
// P30: local PCA normals and a dense projective TSDF for aligned depth maps.
#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <vector>
namespace book03::surface {
using V=std::array<double,3>;
using M=std::array<V,3>;
struct Plane {V normal; double variation;};
inline Plane fit_plane(const std::vector<V>& points){
    if(points.size()<3||points.size()>10000)throw std::invalid_argument("point count");
    V c{}; for(auto p:points)for(int i=0;i<3;++i){if(!std::isfinite(p[i])||std::abs(p[i])>1e6)throw std::invalid_argument("point domain");c[i]+=p[i]/points.size();}
    M a{},v{{{1,0,0},{0,1,0},{0,0,1}}};
    for(auto p:points)for(int i=0;i<3;++i)for(int j=0;j<3;++j)a[i][j]+=(p[i]-c[i])*(p[j]-c[j])/points.size();
    // Symmetric Jacobi similarity rotations preserve eigenvalues.
    for(int sweep=0;sweep<40;++sweep){int i=0,j=1;for(int r=0;r<3;++r)for(int s=r+1;s<3;++s)if(std::abs(a[r][s])>std::abs(a[i][j])){i=r;j=s;}
        double scale=std::abs(a[0][0])+std::abs(a[1][1])+std::abs(a[2][2]);if(std::abs(a[i][j])<=1e-14*scale)break;
        double theta=.5*std::atan2(2*a[i][j],a[j][j]-a[i][i]),co=std::cos(theta),si=std::sin(theta);
        auto rotate=[&](M& m){for(auto& r:m){double x=r[i],y=r[j];r[i]=co*x-si*y;r[j]=si*x+co*y;}};
        rotate(a);for(int k=0;k<3;++k){double x=a[i][k],y=a[j][k];a[i][k]=co*x-si*y;a[j][k]=si*x+co*y;}rotate(v);
    }
    std::array<int,3> order{0,1,2};std::sort(order.begin(),order.end(),[&](int i,int j){return a[i][i]<a[j][j];});int k=order[0];double total=a[0][0]+a[1][1]+a[2][2];
    if(!(total>0)||a[order[1]][order[1]]-a[k][k]<=1e-8*total)throw std::domain_error("ambiguous normal");
    return {{v[0][k],v[1][k],v[2][k]},std::max(0.,a[k][k])/total};
}
struct Cell {double value=0,weight=0;}; // weight zero means unknown, NOT surface
class Volume {
    int nx_,ny_,nz_;double step_,mu_;std::vector<Cell> cells_;
    std::size_t index(int x,int y,int z)const{if(x<0||x>=nx_||y<0||y>=ny_||z<0||z>=nz_)throw std::out_of_range("voxel");return (static_cast<std::size_t>(z)*ny_+y)*nx_+x;}
public:
    Volume(int nx,int ny,int nz,double step,double mu):nx_(nx),ny_(ny),nz_(nz),step_(step),mu_(mu){if(nx<1||ny<1||nz<2||nx>128||ny>128||nz>128||!std::isfinite(step)||!std::isfinite(mu)||step<1e-6||step>1e3||mu<step||mu>1e3)throw std::invalid_argument("volume domain");cells_.resize(static_cast<std::size_t>(nx)*ny*nz);}
    const Cell& at(int x,int y,int z)const{return cells_[index(x,y,z)];}
    double depth(int z)const{return (z+.5)*step_;}
    // Registered orthographic depth along +Z in metres. Missing means no update.
    void integrate(int x,int y,std::optional<double> d,double w=1){(void)index(x,y,0);if(!std::isfinite(w)||w<=0||w>1e6)throw std::invalid_argument("weight");if(!d)return;if(!std::isfinite(*d)||*d<=0||*d>1e6)throw std::invalid_argument("depth");
        for(int z=0;z<nz_;++z){double s=(*d-depth(z))/mu_;if(s< -1)continue;auto& c=cells_[index(x,y,z)];double total=c.weight+w;if(total>1e12)throw std::overflow_error("weight budget");c.value+=(std::clamp(s,-1.,1.)-c.value)*(w/total);c.weight=total;}
    }
    // First positive-to-negative crossing with two observed endpoints.
    std::optional<double> crossing(int x,int y)const{for(int z=0;z<nz_-1;++z){auto a=at(x,y,z),b=at(x,y,z+1);if(a.weight>0&&b.weight>0&&a.value>=0&&b.value<=0&&a.value>b.value)return depth(z)+step_*a.value/(a.value-b.value);}return std::nullopt;}
};
} // namespace book03::surface
