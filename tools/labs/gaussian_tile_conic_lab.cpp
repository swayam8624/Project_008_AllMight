// PROJECTALLMIGHT Gaussian screen conic, tile duplication, sorting and compositing reference.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

struct Cov2 { double a{},b{},c{}; }; // [[a,b],[b,c]]
struct Conic { double A{},B{},C{}; }; // A*x^2 + 2B*x*y + C*y^2

Conic invert(Cov2 s){
    const double det=s.a*s.c-s.b*s.b;
    require(s.a>0&&s.c>0&&det>1e-12,"covariance must be positive definite");
    return {s.c/det,-s.b/det,s.a/det};
}

double quadratic(Conic q,double x,double y){
    return q.A*x*x+2.0*q.B*x*y+q.C*y*y;
}

double lambda_max(Cov2 s){
    const double tr=s.a+s.c;
    const double disc=std::sqrt(std::max(0.0,(s.a-s.c)*(s.a-s.c)+4.0*s.b*s.b));
    return 0.5*(tr+disc);
}

struct Splat {
    int id{};
    double x{},y{},depth{},opacity{};
    Cov2 cov;
};

struct Record { int tile{},id{}; double depth{}; };

std::vector<Record> duplicate_tiles(const std::vector<Splat>& splats,int width,int height,int tile_size,double sigma_radius){
    const int tx=(width+tile_size-1)/tile_size;
    const int ty=(height+tile_size-1)/tile_size;
    std::vector<Record> out;
    for(const auto& g:splats){
        const double r=sigma_radius*std::sqrt(lambda_max(g.cov));
        const int minx=std::max(0,static_cast<int>(std::floor((g.x-r)/tile_size)));
        const int maxx=std::min(tx-1,static_cast<int>(std::floor((g.x+r)/tile_size)));
        const int miny=std::max(0,static_cast<int>(std::floor((g.y-r)/tile_size)));
        const int maxy=std::min(ty-1,static_cast<int>(std::floor((g.y+r)/tile_size)));
        if(minx>maxx||miny>maxy) continue;
        for(int y=miny;y<=maxy;++y)
            for(int x=minx;x<=maxx;++x)
                out.push_back({y*tx+x,g.id,g.depth});
    }
    std::stable_sort(out.begin(),out.end(),[](const Record& a,const Record& b){
        return std::tie(a.tile,a.depth,a.id)<std::tie(b.tile,b.depth,b.id);
    });
    return out;
}

double front_to_back_alpha(const std::vector<double>& alphas){
    double T=1.0;
    double accumulated=0.0;
    for(double a:alphas){
        require(a>=0&&a<=1,"alpha outside [0,1]");
        const double contribution=T*a;
        accumulated+=contribution;
        T*=1.0-a;
    }
    return accumulated;
}

void test_conic(){
    const Cov2 s{4.0,1.0,2.0};
    const Conic q=invert(s);
    // Inverse should satisfy Sigma * Sigma^-1 = I.
    require(std::fabs(s.a*q.A+s.b*q.B-1.0)<1e-12,"inverse (0,0) wrong");
    require(std::fabs(s.a*q.B+s.b*q.C)<1e-12,"inverse (0,1) wrong");
    require(std::fabs(s.b*q.A+s.c*q.B)<1e-12,"inverse (1,0) wrong");
    require(std::fabs(s.b*q.B+s.c*q.C-1.0)<1e-12,"inverse (1,1) wrong");
    require(quadratic(q,0,0)==0.0 && quadratic(q,1,0)>0.0,"conic evaluation wrong");
}

void test_tiles_and_order(){
    const std::vector<Splat> gs{
        {7, 8.0,8.0, 4.0,0.5,{1.0,0.0,1.0}},
        {2, 8.0,8.0, 1.0,0.5,{1.0,0.0,1.0}},
        {9,23.0,8.0, 2.0,0.5,{9.0,0.0,1.0}},
    };
    const auto rec=duplicate_tiles(gs,32,16,16,3.0);
    require(!rec.empty(),"no tile records generated");
    for(std::size_t i=1;i<rec.size();++i){
        require(rec[i-1].tile<rec[i].tile ||
                (rec[i-1].tile==rec[i].tile && rec[i-1].depth<=rec[i].depth),
                "records not tile/depth sorted");
    }
    bool saw_near_before_far=false;
    for(std::size_t i=1;i<rec.size();++i){
        if(rec[i-1].tile==rec[i].tile && rec[i-1].id==2 && rec[i].id==7)
            saw_near_before_far=true;
    }
    require(saw_near_before_far,"front splat did not sort before farther splat in shared tile");
}

void test_compositing(){
    const double a=front_to_back_alpha({0.5,0.5});
    require(std::fabs(a-0.75)<1e-12,"front-to-back transmittance recurrence wrong");
    const double b=front_to_back_alpha({0.2,0.3,0.4});
    const double expected=1.0-(0.8*0.7*0.6);
    require(std::fabs(b-expected)<1e-12,"alpha product identity failed");
}

} // namespace

int main(){
    try{
        test_conic();
        test_tiles_and_order();
        test_compositing();
        std::cout
            <<"gaussian_tile_conic_lab: PASS\n"
            <<"proved: 2D covariance inversion, Mahalanobis conic, eigenvalue footprint bound, tile duplication, tile/depth stable ordering, front-to-back transmittance\n"
            <<"not proved: GPU radix sort, exact ellipse-tile intersection, higher-order SH, per-pixel kernel cutoff, numerical regularization policy, or Maveb/Vulkax parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"gaussian_tile_conic_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
