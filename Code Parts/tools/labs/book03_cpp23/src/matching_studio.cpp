#include "book03/robust_matches.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
using namespace book03;
int main(int argc,char** argv){
    if(argc!=2)return 2;
    std::ofstream out(argv[1]);if(!out)return 1;
    out<<std::setprecision(17)<<"outliers,trial,found,tp,fp,fn,precision,recall,heldout_rms\n";
    std::mt19937_64 rng(2801);std::uniform_real_distribution<double> u(-1,1),px(60,580),py(50,430);std::normal_distribution<double> noise(0,.2);
    auto pixel=[](epipolar::V x){return epipolar::V{800*x[0]/x[2]+320,820*x[1]/x[2]+240,1};};
    auto sample=[&](){epipolar::V x{u(rng),u(rng),4+u(rng)},y{x[0]-.35,x[1]+.03,x[2]+.02};return epipolar::Match{pixel(x),pixel(y)};};
    for(int count:{0,10,20,30,40})for(int trial=0;trial<10;++trial){
        std::vector<epipolar::Match> obs;for(int i=0;i<100;++i){auto m=sample();for(int j=0;j<2;++j){m.a[j]+=noise(rng);m.b[j]+=noise(rng);}if(i<count)m.b={px(rng),py(rng),1};obs.push_back(m);}
        auto result=features::ransac(obs,1,1000,28000+count*10+trial);int tp=0,fp=0;double e=0;
        if(result){for(auto i:result->inliers){if(i<static_cast<std::size_t>(count))++fp;else ++tp;}for(int i=0;i<50;++i)e+=epipolar::sampson(result->f,sample())/50;}
        out<<count<<','<<trial<<','<<bool(result)<<','<<tp<<','<<fp<<','<<100-count-tp<<','<<(tp+fp?double(tp)/(tp+fp):0)<<','<<double(tp)/(100-count)<<','<<(result?std::sqrt(e):-1)<<'\n';
    }return out?0:1;
}
