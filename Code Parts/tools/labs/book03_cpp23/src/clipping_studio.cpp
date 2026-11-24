#include "book03/primitive_clipping.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string_view>
namespace {
using namespace book03;
ClipPolygon fixture(){
    std::array<ProcessedVertex,3> p{};
    p[0].clip={-2,0,.5,1};p[1].clip={0,-.5,.5,1};p[2].clip={0,.5,.5,1};
    p[0].varying.uv=UV2{0,0};p[1].varying.uv=UV2{1,0};p[2].varying.uv=UV2{0,1};
    return clip_triangle(p);
}
void csv(std::ostream& out,const ClipPolygon& p){
    out<<"x,y,z,w,u,v,b0,b1,b2,screen_u,screen_v\n"<<std::setprecision(17);
    for(auto& q:p){auto s=to_screen_vertex(q,{}).screen->image;
        out<<q.clip.x<<","<<q.clip.y<<","<<q.clip.z<<","<<q.clip.w<<","
           <<q.varying.uv->u<<","<<q.varying.uv->v<<","
           <<q.source_weights[0]<<","<<q.source_weights[1]<<","<<q.source_weights[2]<<","<<s.u<<","<<s.v<<"\n";
    }
}
void svg(std::ostream& out,const ClipPolygon& p){
    out<<"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 980 520\" role=\"img\"><title>Clipped triangle</title>"
       <<"<desc>Actual clipped polygon and fan diagonals for a constant-w triangle crossing the left plane.</desc>"
       <<"<rect width=\"980\" height=\"520\" fill=\"#fbfaf7\"/><text x=\"25\" y=\"38\" font-size=\"23\">One outside corner, four surviving vertices</text>"
       <<"<rect x=\"320\" y=\"90\" width=\"600\" height=\"360\" fill=\"#edf3ef\" stroke=\"#507da7\"/>"
       <<"<polygon points=\"20,270 620,360 620,180\" fill=\"#ead9cc\" stroke=\"#b47850\" stroke-width=\"2\"/>";
    auto x=[](const ClipVertex& q){return 620+300*q.clip.x/q.clip.w;};
    auto y=[](const ClipVertex& q){return 270-180*q.clip.y/q.clip.w;};
    out<<"<polygon points=\"";for(auto& q:p)out<<x(q)<<","<<y(q)<<" ";
    out<<"\" fill=\"#c6e4d4\" stroke=\"#367f6b\" stroke-width=\"3\"/>";
    if(p.size()>3)out<<"<line x1=\""<<x(p[0])<<"\" y1=\""<<y(p[0])<<"\" x2=\""<<x(p[2])<<"\" y2=\""<<y(p[2])<<"\" stroke=\"#367f6b\" stroke-dasharray=\"6 4\"/>";
    for(std::size_t i=0;i<p.size();++i)out<<"<circle cx=\""<<x(p[i])<<"\" cy=\""<<y(p[i])<<"\" r=\"5\" fill=\"#367f6b\"/><text x=\""<<x(p[i])+8<<"\" y=\""<<y(p[i])-8<<"\" font-size=\"13\">"<<i<<"</text>";
    out<<"<text x=\"25\" y=\"492\" font-size=\"14\">Brown: input triangle. Green: computed surviving polygon. Dashed: fan diagonal.</text></svg>\n";
}
}
int main(int argc,char** argv){try{
    auto p=fixture();
    if(argc==2&&std::string_view(argv[1])=="inspect"){
        std::cout<<"vertices="<<p.size()<<" triangles="<<fan_triangulate(p).size()<<"\n";csv(std::cout,p);
    }else if(argc==3&&(std::string_view(argv[1])=="csv"||std::string_view(argv[1])=="svg")){
        std::ofstream out(argv[2]);if(!out)throw std::runtime_error("output open failed");
        if(std::string_view(argv[1])=="csv")csv(out,p);else svg(out,p);
        out.flush();if(!out)throw std::runtime_error("output write failed");
    }else{std::cerr<<"book03_clip inspect|csv FILE|svg FILE\n";return 2;}return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
