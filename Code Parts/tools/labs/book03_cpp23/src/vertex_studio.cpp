// P16: import an indexed OBJ witness, trace each referenced tuple, and
// inspect the difference between affine screen and perspective interpolation.
#include "book03/vertex_pipeline.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
namespace {
using namespace book03;
VertexDraw reference(){
    std::istringstream input("v 0 0 -2\nv 2 0 -4\nv 0 4 -8\n"
        "vt 0 0\nvt 1 0\nvt 0 1\nvn 0 0 1\nf 1/1/1 2/2/1 3/3/1\n");
    VertexStage stage({}, {},perspective_matrix({}, {1,20}),{});
    return stage.draw(import_obj(input).to_rendering_mesh());
}
std::array<ProcessedVertex,3> vertices(const VertexDraw& d){
    auto f=d.triangles.at(0).vertex;
    return {*d.vertices.at(f[0]),*d.vertices.at(f[1]),*d.vertices.at(f[2])};
}
void inspect(){
    auto d=reference();auto v=vertices(d);
    std::cout<<std::setprecision(15)<<"invocations="<<d.invocations<<"\n";
    for(std::size_t i=0;i<v.size();++i){
        const auto& p=v[i];
        std::cout<<"vertex "<<i<<" local "<<p.local.x<<","<<p.local.y<<","<<p.local.z
            <<" world "<<p.varying.world.x<<","<<p.varying.world.y<<","<<p.varying.world.z
            <<" camera "<<p.camera.x<<","<<p.camera.y<<","<<p.camera.z
            <<" clip "<<p.clip.x<<","<<p.clip.y<<","<<p.clip.z<<","<<p.clip.w
            <<" screen "<<p.screen->image.u<<","<<p.screen->image.v<<"\n";
    }
    auto p=interpolate_inside_triangle(v,{1./3,1./3,1./3});
    std::cout<<"centroid uv="<<p.varying.uv->u<<","<<p.varying.uv->v
        <<" depth="<<p.depth<<"\n";
}
void csv(std::ostream& out){
    auto v=vertices(reference());
    out<<"t,screen_u,screen_v,affine_u,affine_v,correct_u,correct_v,depth\n"<<std::setprecision(17);
    // Along the screen edge from vertex 1 to vertex 2.
    for(int i=0;i<=100;++i){
        double t=i/100.;
        auto p=interpolate_inside_triangle(v,{0,1-t,t});
        auto a=v[1].screen->image,b=v[2].screen->image;
        out<<t<<","<<std::lerp(a.u,b.u,t)<<","<<std::lerp(a.v,b.v,t)
            <<","<<1-t<<","<<t<<","<<p.varying.uv->u<<","<<p.varying.uv->v<<","<<p.depth<<"\n";
    }
}
void svg(std::ostream& out){
    auto v=vertices(reference());
    const auto xy=[&](std::size_t i){auto p=v[i].screen->image;return std::array<double,2>{p.u-180,p.v+100};};
    out<<"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 980 520\" role=\"img\">"
        "<title>Projected triangle and reciprocal-w edge interpolation</title>"
        "<desc>The left triangle uses actual projected screen vertices; the right graph compares screen-affine and perspective-correct V along one edge.</desc>"
        "<rect width=\"980\" height=\"520\" fill=\"#fbfaf7\"/>"
        "<text x=\"30\" y=\"35\" font-family=\"sans-serif\" font-size=\"22\">One triangle, two interpolation laws</text><polygon points=\"";
    for(std::size_t i=0;i<3;++i){auto p=xy(i);out<<p[0]<<","<<p[1]<<" ";}
    out<<"\" fill=\"#d7e9df\" stroke=\"#397d6b\" stroke-width=\"3\"/>";
    for(std::size_t i=0;i<3;++i){auto p=xy(i);out<<"<text x=\""<<p[0]+8<<"\" y=\""<<p[1]-8<<"\" font-size=\"15\">w="<<v[i].clip.w<<"</text>";}
    out<<"<line x1=\"500\" y1=\"430\" x2=\"930\" y2=\"430\" stroke=\"#6c827c\"/>"
        "<line x1=\"500\" y1=\"430\" x2=\"500\" y2=\"100\" stroke=\"#6c827c\"/>"
        "<text x=\"500\" y=\"85\" font-size=\"15\">V attribute</text>";
    for(bool correct:{false,true}){
        out<<"<polyline fill=\"none\" stroke=\""<<(correct?"#397d6b":"#b47850")<<"\" stroke-width=\"3\" points=\"";
        for(int i=0;i<=100;++i){
            double t=i/100.;
            double a=correct?interpolate_inside_triangle(v,{0,1-t,t}).varying.uv->v:t;
            out<<500+430*t<<","<<430-330*a<<" ";
        }out<<"\"/>";
    }
    out<<"<text x=\"500\" y=\"463\" font-size=\"14\">Screen edge parameter: 0 to 1</text>"
        "<text x=\"30\" y=\"497\" font-family=\"sans-serif\" font-size=\"13\">Brown: screen-affine V. Green: reciprocal-w V. Computed scalar samples; no rasterization.</text></svg>\n";
}
}
int main(int argc,char** argv){
    try{
        if(argc==2&&std::string_view(argv[1])=="inspect")inspect();
        else if(argc==3&&(std::string_view(argv[1])=="csv"||std::string_view(argv[1])=="svg")){
            std::ofstream out(argv[2]);if(!out)throw std::runtime_error("output open failed");
            if(std::string_view(argv[1])=="csv")csv(out);else svg(out);
            out.flush();if(!out)throw std::runtime_error("output write failed");
        }else{std::cerr<<"book03_vertex inspect|csv FILE|svg FILE\n";return 2;}
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
