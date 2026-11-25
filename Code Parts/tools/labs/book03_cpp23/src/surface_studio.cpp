#include "book03/surface.hpp"
#include <fstream>
#include <iomanip>
#include <random>
using namespace book03::surface;
int main(int argc,char** argv){if(argc!=2)return 2;std::ofstream out(argv[1]);if(!out)return 1;out<<std::setprecision(17)<<"frames,trial,first_rmse,fused_rmse,observed_columns,missing_columns\n";std::mt19937_64 rng(30);std::normal_distribution<double> noise(0,.015);
for(int frames:{1,2,4,8,16})for(int trial=0;trial<20;++trial){Volume v(16,16,100,.02,.12);double a=0,b=0;int count=0,missing=0;for(int y=0;y<16;++y)for(int x=0;x<16;++x){if((x+y)%7==0){++missing;continue;}double d=.8+.01*x+.005*y;double first=d+noise(rng);v.integrate(x,y,first);a+=(first-d)*(first-d);for(int k=1;k<frames;++k)v.integrate(x,y,d+noise(rng));double e=v.crossing(x,y).value()-d;b+=e*e;++count;}out<<frames<<','<<trial<<','<<std::sqrt(a/count)<<','<<std::sqrt(b/count)<<','<<count<<','<<missing<<'\n';}
return out?0:1;}
