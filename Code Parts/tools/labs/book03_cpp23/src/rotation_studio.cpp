// P12 orientation debugger. All displayed basis arrows and CSV samples
// come from the actual quaternion implementation. No camera/visibility.
#include "book03/rotation3.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
namespace {
using namespace book03;
constexpr double deg=std::numbers::pi/180;
void inspect(Rotation3 q){
    const auto p=q.components();const auto M=q.matrix();
    const auto e=principal_fixed_xyz(q);
    std::cout<<std::setprecision(12)<<"q(wxyz)="<<p.w<<","<<p.x<<","<<p.y<<","<<p.z
        <<"\ndet="<<M.determinant()<<" fixed-XYZ degrees="
        <<e.x/deg<<","<<e.y/deg<<","<<e.z/deg
        <<" near-lock="<<(e.near_lock?"yes":"no")<<"\n";
    for(auto v:{M.x,M.y,M.z})std::cout<<"basis "<<v.x<<","<<v.y<<","<<v.z<<"\n";
}
void csv(std::ostream& out,Rotation3 a,Rotation3 b){
    out<<"t,w,x,y,z,tip_x,tip_y,tip_z,det\n"<<std::setprecision(17);
    for(int i=0;i<=60;++i){
        const double t=i/60.0;const auto q=Rotation3::slerp(a,b,t);
        const auto p=q.components();const auto v=q.apply({1,0,0});
        out<<t<<","<<p.w<<","<<p.x<<","<<p.y<<","<<p.z<<","
            <<v.x<<","<<v.y<<","<<v.z<<","<<q.matrix().determinant()<<"\n";
    }
}
void svg(std::ostream& out,Rotation3 a,Rotation3 b){
    out<<"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 1080 560\" role=\"img\">"
        "<title>Rotation interpolation basis frames</title>"
        "<desc>Five sampled short-arc quaternion orientations, oblique projection of actual rotated basis vectors.</desc>"
        "<rect width=\"1080\" height=\"560\" fill=\"#fbfaf6\"/>"
        "<text x=\"30\" y=\"42\" font-family=\"sans-serif\" font-size=\"24\">Orientation frames along an interpolation</text>";
    for(int i=0;i<=4;++i){
        const double t=i/4.0,cx=130+205*i,cy=300;
        const auto M=Rotation3::slerp(a,b,t).matrix();
        const std::array<Vector3,3> axes{M.x,M.y,M.z};
        const std::array<const char*,3> colors{"#b16b51","#36806c","#527eac"};
        out<<"<circle cx=\""<<cx<<"\" cy=\""<<cy<<"\" r=\"85\" fill=\"#edf2ed\"/>";
        for(int k=0;k<3;++k){
            const auto v=axes[k];const auto x=cx+65*(v.x-.5*v.z),y=cy+65*(-v.y+.3*v.z);
            out<<"<line x1=\""<<cx<<"\" y1=\""<<cy<<"\" x2=\""<<x<<"\" y2=\""<<y
               <<"\" stroke=\""<<colors[k]<<"\" stroke-width=\"4\"/>"
               <<"<circle cx=\""<<x<<"\" cy=\""<<y<<"\" r=\"4\" fill=\""<<colors[k]<<"\"/>";
        }
        out<<"<text x=\""<<cx-25<<"\" y=\"420\" font-family=\"sans-serif\" font-size=\"15\">t="<<t<<"</text>";
    }
    out<<"<text x=\"30\" y=\"520\" font-family=\"sans-serif\" font-size=\"14\">X: terracotta | Y: green | Z: blue. Parallel illustrative projection.</text></svg>\n";
}
void save(std::string path,bool picture,Rotation3 a,Rotation3 b){
    std::ofstream out(path);
    if(!out)throw std::runtime_error("output open failed");
    if(picture)svg(out,a,b);else csv(out,a,b);
    out.flush();if(!out)throw std::runtime_error("output write failed");
}
void repl(){
    Rotation3 a{},b=Rotation3::axis_angle({0,0,1},120*deg);
    std::cout<<"axis X Y Z DEGREES | euler X Y Z (degrees) | start | sample T | inspect | csv FILE | svg FILE | quit\n";
    std::string line;
    while(std::getline(std::cin,line)){
        std::istringstream in(line);std::string cmd;in>>cmd;
        try{
            if(cmd=="quit")break;
            if(cmd=="axis"){
                Vector3 v;double d;
                if(!(in>>v.x>>v.y>>v.z>>d))throw std::invalid_argument("axis syntax");
                b=Rotation3::axis_angle(v,d*deg);
            }else if(cmd=="euler"){
                double x,y,z;if(!(in>>x>>y>>z))throw std::invalid_argument("euler syntax");
                b=Rotation3::fixed_xyz(x*deg,y*deg,z*deg);
            }else if(cmd=="start")a=b;
            else if(cmd=="sample"){
                double t;if(!(in>>t))throw std::invalid_argument("sample syntax");
                inspect(Rotation3::slerp(a,b,t));
            }else if(cmd=="inspect")inspect(b);
            else if(cmd=="csv"||cmd=="svg"){
                std::string file;if(!(in>>file))throw std::invalid_argument("missing path");
                save(file,cmd=="svg",a,b);
            }else if(!cmd.empty())std::cout<<"Unknown command\n";
        }catch(const std::exception& e){std::cout<<"Rejected: "<<e.what()<<"\n";}
    }
}
}
int main(int argc,char** argv){
    try{
        const Rotation3 a{},b=Rotation3::axis_angle({0,0,1},120*deg);
        if(argc==2&&std::string_view(argv[1])=="inspect")inspect(b);
        else if(argc==2&&std::string_view(argv[1])=="repl")repl();
        else if(argc==3&&(std::string_view(argv[1])=="csv"||std::string_view(argv[1])=="svg"))
            save(argv[2],std::string_view(argv[1])=="svg",a,b);
        else{std::cerr<<"book03_rotation inspect|repl|csv FILE|svg FILE\n";return 2;}
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
