#pragma once
// P28: integer pixel indices; grayscale intensity in [0,1].
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>
namespace book03::features {
struct Image {
    int w,h; std::vector<double> data;
    Image(int width,int height):w(width),h(height) {
        if(w<1||h<1||w>2048||h>2048)throw std::invalid_argument("image size");
        data.resize(static_cast<std::size_t>(w)*h);
    }
    double& at(int x,int y){if(x<0||y<0||x>=w||y>=h)throw std::out_of_range("pixel");return data[static_cast<std::size_t>(y)*w+x];}
    double at(int x,int y)const{if(x<0||y<0||x>=w||y>=h)throw std::out_of_range("pixel");return data[static_cast<std::size_t>(y)*w+x];}
};
struct Corner {int x,y;double response;};
inline std::vector<Corner> harris(const Image& im,double threshold){
    if(!std::isfinite(threshold)||threshold<=0)throw std::invalid_argument("threshold");
    for(double v:im.data)if(!std::isfinite(v)||v<0||v>1)throw std::invalid_argument("intensity");
    Image score(im.w,im.h);
    // Central differences and a uniform 3x3 structure-tensor window.
    for(int y=2;y<im.h-2;++y)for(int x=2;x<im.w-2;++x){double a=0,b=0,c=0;
        for(int j=-1;j<=1;++j)for(int i=-1;i<=1;++i){double dx=(im.at(x+i+1,y+j)-im.at(x+i-1,y+j))/2,dy=(im.at(x+i,y+j+1)-im.at(x+i,y+j-1))/2;a+=dx*dx;b+=dx*dy;c+=dy*dy;}
        score.at(x,y)=a*c-b*b-.04*(a+c)*(a+c);
    }
    std::vector<Corner> out;
    // Lexicographic ties: reject a tied neighbour with an earlier index.
    for(int y=2;y<im.h-2;++y)for(int x=2;x<im.w-2;++x){double s=score.at(x,y);if(s<threshold)continue;bool keep=true;
        for(int j=-1;j<=1;++j)for(int i=-1;i<=1;++i){double t=score.at(x+i,y+j);if(t>s||(t==s&&(j<0||(j==0&&i<0))))keep=false;}
        if(keep)out.push_back({x,y,s});
    }return out;
}
using Descriptor=std::array<double,49>;
inline std::optional<Descriptor> describe(const Image& im,int x,int y){
    if(x<3||y<3||x>=im.w-3||y>=im.h-3)return std::nullopt;
    Descriptor d{};double mean=0;int k=0;
    for(int j=-3;j<=3;++j)for(int i=-3;i<=3;++i){double v=im.at(x+i,y+j);if(!std::isfinite(v)||v<0||v>1)throw std::invalid_argument("intensity");d[k++]=v;mean+=v/49;}
    double norm2=0;for(auto& v:d){v-=mean;norm2+=v*v;}if(norm2<=1e-20)return std::nullopt;
    for(auto& v:d)v/=std::sqrt(norm2);
    return d;
}
inline double distance2(const Descriptor& a,const Descriptor& b){double s=0;for(std::size_t i=0;i<a.size();++i){double d=a[i]-b[i];s+=d*d;}if(!std::isfinite(s))throw std::invalid_argument("descriptor");return s;}
struct Pair {std::size_t a,b;double squared_distance;};
inline std::vector<Pair> match(const std::vector<Descriptor>& a,const std::vector<Descriptor>& b,double ratio=.8){
    if(!(ratio>0&&ratio<1))throw std::invalid_argument("ratio");
    std::vector<Pair> out;if(a.empty()||b.size()<2)return out;
    for(std::size_t i=0;i<a.size();++i){double first=std::numeric_limits<double>::infinity(),second=first;std::size_t best=0;
        for(std::size_t j=0;j<b.size();++j){double d=distance2(a[i],b[j]);if(d<first){second=first;first=d;best=j;}else if(d<second)second=d;}
        if(!(first<ratio*ratio*second))continue; // squared distances; ties fail
        bool mutual=true;for(std::size_t j=0;j<a.size();++j)if(j!=i&&distance2(a[j],b[best])<=first)mutual=false;
        if(mutual)out.push_back({i,best,first});
    }return out;
}
} // namespace book03::features
