#include "book03/features.hpp"
#include "book03/robust_matches.hpp"
#include <iostream>
using namespace book03::features;
void check(bool v){if(!v)throw std::runtime_error("feature contract");}
int main(){
    Image a(64,64),b(64,64),flat(64,64),edge(64,64);
    std::mt19937_64 rng(28);std::uniform_real_distribution<double> u(0,1);
    for(int y=0;y<64;++y)for(int x=0;x<64;++x){a.at(x,y)=u(rng);edge.at(x,y)=x>=32?1:0;}
    for(int y=0;y<61;++y)for(int x=0;x<59;++x)b.at(x+5,y+3)=.2+.6*a.at(x,y);
    check(harris(flat,.001).empty()&&harris(edge,.001).empty()&&!describe(flat,20,20)&&!describe(a,1,1));
    auto ca=harris(a,.001),cb=harris(b,.001*.6*.6*.6*.6);std::vector<Descriptor> da,db;std::vector<Corner> pa,pb;
    for(auto p:ca)if(p.x>=8&&p.x<51&&p.y>=8&&p.y<53){da.push_back(*describe(a,p.x,p.y));pa.push_back(p);}
    for(auto p:cb)if(p.x>=13&&p.x<56&&p.y>=11&&p.y<56){db.push_back(*describe(b,p.x,p.y));pb.push_back(p);}
    auto pairs=match(da,db);check(pairs.size()>100&&pairs.size()==pa.size());
    for(auto p:pairs)check(pb[p.b].x-pa[p.a].x==5&&pb[p.b].y-pa[p.a].y==3&&p.squared_distance<1e-25);
    check(match({da[0]},{da[0],da[0]}).empty());
    // Planar/repeated correspondences must not manufacture a consensus model.
    std::vector<book03::epipolar::Match> bad(8,{{1,2,1},{2,3,1}});check(!ransac(bad,1,10,4));
    std::cout<<"P28 PASS translated matches="<<pairs.size()<<'\n';
}
