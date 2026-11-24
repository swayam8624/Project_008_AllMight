#include "book03/thin_lens.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string_view>
using namespace book03;
int main(int argc,char** argv){try{
    if(argc!=3||std::string_view(argv[1])!="csv")throw std::invalid_argument("book03_lens csv FILE");
    std::ofstream out(argv[2]);if(!out)throw std::runtime_error("open failed");
    out<<"f_number,object_mm,blur_mm,ray_diameter_mm,blur_pixels,airy_mm\n"<<std::setprecision(17);
    for(double n:{2.,4.,8.,16.})for(int i=0;i<=100;++i){
        ThinLens l{50,n,3000};double z=500+95*i,r=l.aperture_mm()/2;
        auto a=l.sensor_hit({0,0},z,{r,0}),b=l.sensor_hit({0,0},z,{-r,0});
        double blur=l.blur_mm(z);
        out<<n<<','<<z<<','<<blur<<','<<std::abs(a.x-b.x)<<','<<blur/.004<<','<<l.airy_infinity_mm(.00055)<<'\n';
    }
    out.flush();if(!out)throw std::runtime_error("write failed");return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
