#include "book03/studio.hpp"
#include <iostream>
#include <sstream>
using namespace book03;
using namespace book03::studio;
void check(bool b){if(!b)throw std::runtime_error("studio contract");}
int main(){
    auto truth=house();auto rec=reconstruct(truth);check(rec.mesh.points.size()==9&&rec.tracks.size()==9&&rec.max_3d<1e-10&&rec.rms_pixels<1e-10);
    auto plane=surface::fit_plane({rec.mesh.points[0],rec.mesh.points[1],rec.mesh.points[2],rec.mesh.points[3]});check(std::abs(plane.normal[2])>1-1e-12);
    auto a=render(truth),b=render(rec.mesh),c=render(rec.mesh,true);std::array<int,4> counts{};
    for(int y=0;y<height;++y)for(int x=0;x<width;++x){double u=x+.5,v=y+.5;unsigned id=0;
        if(v>=40&&v<100&&std::abs(u-160)<(v-40)*4/3)id=2;
        if(u>=80&&u<240&&v>=100&&v<200)id=1;
        if(u>=130&&u<190&&v>=135&&v<175)id=3;
        ++counts[id];for(auto* image:{&a,&b,&c}){auto winner=image->winner(x,y);check(bool(winner)==(id!=0));if(id){check(winner->id==id);double z=id==3?3.8:4;check(std::abs(winner->depth-(10./9)*(1-1/z))<1e-10);}auto col=image->color(x,y);auto expected=id==1?RenderColor{.65,.43,.22}:id==2?RenderColor{.50,.12,.08}:id==3?RenderColor{.08,.40,.65}:RenderColor{.035,.05,.065};check(col.r==expected.r&&col.g==expected.g&&col.b==expected.b);}}
    check(a.stats().triangles==5&&b.stats().degenerate==0&&c.stats().failed>0);
    std::ostringstream p,q;a.ppm(p);b.ppm(q);check(p.str()==q.str());
    try{(void)vertex({0,0,.5});check(false);}catch(const std::domain_error&){}
    try{(void)vertex({20,0,4});check(false);}catch(const std::domain_error&){}
    truth.faces[0].vertices[0]=999;try{(void)render(truth);check(false);}catch(const std::out_of_range&){}
    std::cout<<"P31 PASS max_3d="<<rec.max_3d<<" rms_pixels="<<rec.rms_pixels<<" pixels=";for(auto n:counts)std::cout<<n<<',';std::cout<<'\n';
}
