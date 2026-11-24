#include "book03/sensor.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
using namespace book03::sensor;
int main(int argc,char** argv) {
    if(argc!=2) {std::cerr<<"usage: book03_sensor output.csv\n";return 2;}
    std::ofstream out(argv[1]); if(!out) return 1;
    out<<std::setprecision(17)<<"incident,signal,dark,variance,snr,mean_analog,variance_analog,mean_dn,well_clip_fraction,adc_top_fraction\n";
    Config p; std::mt19937_64 rng(2401); constexpr int n=10000;
    for(int stop=-8;stop<=8;++stop) {
        const double incident=1000*std::exp2(stop); const auto m=moments(p,incident,0.01);
        double sum=0,sq=0,dn=0; int well=0,top=0;
        for(int i=0;i<n;++i) {const auto a=capture(p,incident,0.01,rng);sum+=a.analog;sq+=a.analog*a.analog;dn+=a.dn;well+=a.charge>p.full_well;top+=a.dn==4095;}
        const double mean=sum/n;
        out<<incident<<','<<m.signal<<','<<m.dark<<','<<m.variance<<','<<m.snr<<','<<mean<<','<<sq/n-mean*mean<<','<<dn/n<<','<<double(well)/n<<','<<double(top)/n<<'\n';
    }
    return out?0:1;
}
