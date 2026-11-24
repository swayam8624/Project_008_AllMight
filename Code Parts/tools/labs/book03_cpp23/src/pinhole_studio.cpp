// P13 pinhole simulator. Point projection is NOT triangle rasterization:
// we emit landmarks, their continuous image coordinates and ray directions.
#include "book03/pinhole_camera.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
namespace {
using namespace book03;
struct Landmark {const char* name;Point3 p;};
constexpr std::array<Landmark,7> house{{
    {"base-left",{-1,0,-5}},{"base-right",{1,0,-5}},
    {"wall-right",{1,1.6,-5}},{"wall-left",{-1,1.6,-5}},
    {"roof",{0,2.6,-5}},{"tree",{2,2.5,-7}},{"ball",{1,-0.5,-4}}
}};
void report(const PinholeCamera& c){
    std::cout<<std::setprecision(12);
    for(const auto& l:house){
        try{const auto p=c.project(l.p);
            std::cout<<l.name<<" u="<<p.image.u<<" v="<<p.image.v
                <<" axial="<<p.axial_depth<<" inside="<<(p.inside_image?"yes":"no")<<"\n";
        }catch(const std::domain_error&){std::cout<<l.name<<" behind/at camera\n";}
    }
}
void csv(std::ostream& out,const PinholeCamera& c){
    out<<"name,X_world,Y_world,Z_world,u,v,axial_depth,inside,ray_x,ray_y,ray_z\n"
       <<std::setprecision(17);
    for(const auto& l:house){
        try{const auto p=c.project(l.p);const auto r=c.ray(p.image);
            out<<l.name<<","<<l.p.x<<","<<l.p.y<<","<<l.p.z<<","
                <<p.image.u<<","<<p.image.v<<","<<p.axial_depth<<","
                <<(p.inside_image?1:0)<<","<<r.direction.x<<","<<r.direction.y<<","<<r.direction.z<<"\n";
        }catch(const std::domain_error&){/* No projected sample at/behind pinhole. */}
    }
}
void svg(std::ostream& out,const PinholeCamera& c){
    const auto w=c.intrinsics.width,h=c.intrinsics.height;
    const double s=std::min(740.0/static_cast<double>(w),430.0/static_cast<double>(h));
    out<<"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 900 560\" role=\"img\">"
        "<title>Ideal pinhole landmark projection</title><desc>Actual projected landmarks inside the image. No triangle coverage, distortion or visibility calculation.</desc>"
        "<rect width=\"900\" height=\"560\" fill=\"#fbfaf6\"/>"
        "<text x=\"25\" y=\"35\" font-family=\"sans-serif\" font-size=\"22\">Projected landmarks: continuous image coordinates</text>"
        "<rect x=\"40\" y=\"70\" width=\""<<w*s<<"\" height=\""<<h*s<<"\" fill=\"#e9f1eb\" stroke=\"#607e7c\"/>";
    for(const auto& l:house){
        try{const auto p=c.project(l.p);if(!p.inside_image)continue;
            const double x=40+s*p.image.u,y=70+s*p.image.v;
            out<<"<circle cx=\""<<x<<"\" cy=\""<<y<<"\" r=\"5\" fill=\"#397d6c\"/>"
                <<"<text x=\""<<x+8<<"\" y=\""<<y-7<<"\" font-family=\"sans-serif\" font-size=\"12\">"<<l.name<<"</text>";
        }catch(const std::domain_error&){}
    }
    out<<"<text x=\"25\" y=\"535\" font-family=\"sans-serif\" font-size=\"13\">Point samples only; no occlusion, raster coverage, lens distortion or clipping.</text></svg>\n";
}
void save(const PinholeCamera& c,const std::string& path,bool picture){
    std::ofstream out(path);if(!out)throw std::runtime_error("cannot open output");
    if(picture)svg(out,c);else csv(out,c);
    out.flush();if(!out)throw std::runtime_error("output failed");
}
void repl(){
    PinholeCamera c;
    std::cout<<"inspect | move X Y Z | yaw DEGREES | focal FX FY | principal CX CY | ray I J | csv FILE | svg FILE | reset | quit\n";
    std::string line;
    while(std::getline(std::cin,line)){
        std::istringstream in(line);std::string cmd;in>>cmd;
        try{
            if(cmd=="quit")break;
            if(cmd=="inspect")report(c);
            else if(cmd=="reset")c={};
            else if(cmd=="move"){
                Vector3 p;if(!(in>>p.x>>p.y>>p.z)||!finite(p))throw std::invalid_argument("move syntax");
                c.camera_to_world.translation=p;
            }else if(cmd=="yaw"){
                double a;if(!(in>>a))throw std::invalid_argument("yaw syntax");
                c.camera_to_world.rotation=Rotation3::axis_angle({0,1,0},a*(std::numbers::pi/180));
            }else if(cmd=="focal"||cmd=="principal"){
                auto k=c.intrinsics;double a,b;if(!(in>>a>>b))throw std::invalid_argument("two values required");
                if(cmd=="focal"){k.fx=a;k.fy=b;}else{k.cx=a;k.cy=b;}
                k.validate();c.intrinsics=k;
            }else if(cmd=="ray"){
                std::size_t i,j;if(!(in>>i>>j))throw std::invalid_argument("ray syntax");
                const auto r=c.pixel_ray(i,j);
                std::cout<<std::setprecision(12)<<"ray "<<r.direction.x<<","<<r.direction.y<<","<<r.direction.z<<"\n";
            }else if(cmd=="svg"||cmd=="csv"){
                std::string path;if(!(in>>path))throw std::invalid_argument("missing output");
                save(c,path,cmd=="svg");
            }else if(!cmd.empty())std::cout<<"Unknown command\n";
        }catch(const std::exception& e){std::cout<<"Rejected: "<<e.what()<<"\n";}
    }
}
}
int main(int argc,char** argv){
    try{
        PinholeCamera c;
        if(argc==2&&std::string_view(argv[1])=="inspect")report(c);
        else if(argc==2&&std::string_view(argv[1])=="repl")repl();
        else if(argc==3&&(std::string_view(argv[1])=="svg"||std::string_view(argv[1])=="csv"))
            save(c,argv[2],std::string_view(argv[1])=="svg");
        else{std::cerr<<"book03_camera inspect|repl|svg FILE|csv FILE\n";return 2;}
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
