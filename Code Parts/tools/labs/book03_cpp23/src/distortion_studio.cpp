#include "book03/distortion.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
using namespace book03;
int main(int argc,char** argv){try{
    if(argc!=2)throw std::invalid_argument("book03_distortion OUTPUT.csv");
    std::ofstream out(argv[1]);if(!out)throw std::runtime_error("open failed");
    BrownConrady m{-.18,.04,-.005,.004,-.003,1};
    out<<"x,y,xd,yd,recovered_x,recovered_y,residual,iterations\n"<<std::setprecision(17);
    for(int j=-8;j<=8;++j)for(int i=-8;i<=8;++i){
        DistortionPoint p{i/10.,j/10.};if(std::hypot(p.x,p.y)>.98)continue;
        auto d=m.forward(p);auto q=undistort(m,d);
        if(q.status!=InverseStatus::Converged)throw std::runtime_error("inverse did not converge");
        out<<p.x<<','<<p.y<<','<<d.x<<','<<d.y<<','<<q.point.x<<','<<q.point.y<<','<<q.residual<<','<<q.iterations<<'\n';
    }
    out.flush();if(!out)throw std::runtime_error("write failed");return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
