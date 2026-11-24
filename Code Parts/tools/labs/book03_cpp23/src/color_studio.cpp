#include "book03/color.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
using namespace book03::color;
int main(int argc,char** argv){
    if(argc!=2){std::cerr<<"usage: book03_color output.csv\n";return 2;}
    std::ofstream out(argv[1]);if(!out)return 1;
    out<<std::setprecision(17)<<"linear_r,linear_g,linear_b,X,Y,Z,p3_r,p3_g,p3_b,srgb_encoded_r,srgb_encoded_g,srgb_encoded_b\n";
    for(int r=0;r<=8;++r)for(int g=0;g<=8;++g)for(int b=0;b<=8;++b){
        Triple a{r/8.,g/8.,b/8.};auto xyz=mul(srgb,a);auto p=convert(a,srgb,p3);
        for(auto v:{a,xyz,p})for(double x:v)out<<x<<',';
        out<<encode(a[0])<<','<<encode(a[1])<<','<<encode(a[2])<<'\n';
    }
    return out?0:1;
}
