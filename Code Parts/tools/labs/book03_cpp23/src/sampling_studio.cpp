#include "book03/sampling.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string_view>
using namespace book03;
int main(int argc,char** argv){try{
    if(argc!=3||std::string_view(argv[1])!="csv")throw std::invalid_argument("book03_sampling csv FILE");
    std::ofstream out(argv[2]);if(!out)throw std::runtime_error("open failed");
    out<<"frequency,phase,pixel,point,ss4,ss16,box\n"<<std::setprecision(17);
    for(double f:{.2,.6,1.2,2.,3.7})for(double phase:{0.,.7,1.4})for(int i=0;i<64;++i){
        Wave s{f,phase};double x=i+.5;
        out<<f<<","<<phase<<","<<i<<","<<s.at(x)<<","<<midpoint_box(s,x,4)<<","<<midpoint_box(s,x,16)<<","<<s.box(x)<<"\n";
    }
    out.flush();if(!out)throw std::runtime_error("write failed");return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
