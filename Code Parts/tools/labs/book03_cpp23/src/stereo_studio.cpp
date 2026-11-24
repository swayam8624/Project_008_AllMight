// P15: a numerical stereo viewer using known synthetic correspondences.
// No feature detector, matching, rectification warp or image reconstruction.
#include "book03/stereo_geometry.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
namespace {
using namespace book03;
constexpr std::array<Point3,5> house{{
    {-1,0,-5},{1,0,-5},{1,1.6,-5},{-1,1.6,-5},{0,2.6,-5}}};
void inspect(const RectifiedStereo& rig,Point3 p,double noise){
    const auto l=rig.left_camera(),r=rig.right_camera();
    const auto a=l.project(p).image;
    auto b=r.project(p).image;b.u+=noise;
    const auto t=triangulate_samples(l,r,a,b);
    std::cout<<std::setprecision(15)<<"left "<<a.u<<","<<a.v
        <<" right "<<b.u<<","<<b.v<<"\ncorrected disparity="
        <<rig.disparity(a,b)<<" row residual="<<rig.row_residual(a,b)
        <<"\nsin angle="<<t.geometry.sin_angle<<"\n";
    if(t.geometry.point){
        const auto q=*t.geometry.point;
        std::cout<<"estimate "<<q.x<<","<<q.y<<","<<q.z
            <<" gap="<<t.geometry.ray_gap<<" reprojection="
            <<*t.left_pixel_error<<","<<*t.right_pixel_error<<"\n";
    }else std::cout<<"Rejected: weak parallax or behind camera\n";
}
void csv(std::ostream& out,const RectifiedStereo& rig){
    out<<"depth,disparity,sigma_depth,low_depth,high_depth\n"<<std::setprecision(17);
    constexpr double sigma=.25;
    for(int i=0;i<=100;++i){
        const double z=1+0.19*i;
        const Point3 p=rig.left_pose.point({0,0,-z});
        const auto a=rig.left_camera().project(p).image,b=rig.right_camera().project(p).image;
        const double d=rig.disparity(a,b);
        out<<z<<","<<d<<","<<rig.depth_sigma(d,sigma)<<","
           <<rig.axial_depth(d+sigma)<<",";
        if(d>sigma)out<<rig.axial_depth(d-sigma);
        out<<"\n";
    }
}
void svg(std::ostream& out,const RectifiedStereo& rig){
    out<<"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 1080 560\" role=\"img\">"
        "<title>Two projections of one house</title>"
        "<desc>Actual synthetic corresponding points in a rectified stereo pair; coordinates scaled into separate panels. No image matching or occlusion simulation.</desc>"
        "<rect width=\"1080\" height=\"560\" fill=\"#fbfaf7\"/>"
        "<text x=\"30\" y=\"38\" font-family=\"sans-serif\" font-size=\"24\">Calibrated left and right image measurements</text>";
    const auto l=rig.left_camera(),r=rig.right_camera();
    for(int eye=0;eye<2;++eye){
        const auto& c=eye?r:l;
        const double x=25+530*eye,y=80,s=.74;
        out<<"<rect x=\""<<x<<"\" y=\""<<y<<"\" width=\""<<640*s
           <<"\" height=\""<<480*s<<"\" fill=\"#e8f0e9\" stroke=\"#62857c\"/>";
        out<<"<text x=\""<<x<<"\" y=\"460\" font-family=\"sans-serif\" font-size=\"16\">"
           <<(eye?"Right":"Left")<<" camera</text>";
        std::array<ImageSample,5> p{};
        for(std::size_t i=0;i<house.size();++i)p[i]=c.project(house[i]).image;
        constexpr std::array<std::array<int,2>,6> edges{{{0,1},{1,2},{2,3},{3,0},{3,4},{4,2}}};
        for(auto e:edges)
            out<<"<line x1=\""<<x+s*p[e[0]].u<<"\" y1=\""<<y+s*p[e[0]].v
               <<"\" x2=\""<<x+s*p[e[1]].u<<"\" y2=\""<<y+s*p[e[1]].v
               <<"\" stroke=\"#387e6e\" stroke-width=\"2.5\"/>";
        for(std::size_t i=0;i<p.size();++i){
            out<<"<circle cx=\""<<x+s*p[i].u<<"\" cy=\""<<y+s*p[i].v
               <<"\" r=\"5\" fill=\"#b77852\"/><text x=\""<<x+s*p[i].u+8
               <<"\" y=\""<<y+s*p[i].v-8<<"\" font-size=\"12\">"<<i<<"</text>";
        }
    }
    out<<"<text x=\"25\" y=\"535\" font-family=\"sans-serif\" font-size=\"13\">Known correspondences, not a stereo matching result. Baseline "<<rig.baseline<<" m.</text></svg>\n";
}
void save(const RectifiedStereo& rig,const std::string& path,bool image){
    std::ofstream out(path);if(!out)throw std::runtime_error("cannot open output");
    if(image)svg(out,rig);else csv(out,rig);
    out.flush();if(!out)throw std::runtime_error("output failed");
}
void repl(){
    RectifiedStereo rig;Point3 p{1,1,-4};double noise=0;
    std::cout<<"point X Y Z | baseline B | noise PIXELS | inspect | csv FILE | svg FILE | quit\n";
    std::string line;
    while(std::getline(std::cin,line)){
        std::istringstream in(line);std::string cmd;in>>cmd;
        try{
            if(cmd=="quit")break;
            if(cmd=="inspect")inspect(rig,p,noise);
            else if(cmd=="point"){
                Point3 q;if(!(in>>q.x>>q.y>>q.z)||!finite(q))throw std::invalid_argument("point syntax");
                p=q;
            }else if(cmd=="baseline"){
                double b;if(!(in>>b)||b<.01||b>1)throw std::invalid_argument("studio baseline range [.01,1]");
                rig.baseline=b;
            }else if(cmd=="noise"){
                double e;if(!(in>>e)||!std::isfinite(e))throw std::invalid_argument("noise syntax");
                noise=e;
            }else if(cmd=="csv"||cmd=="svg"){
                std::string pth;if(!(in>>pth))throw std::invalid_argument("missing path");
                save(rig,pth,cmd=="svg");
            }else if(!cmd.empty())std::cout<<"Unknown command\n";
        }catch(const std::exception& e){std::cout<<"Rejected: "<<e.what()<<"\n";}
    }
}
}
int main(int argc,char** argv){
    try{
        RectifiedStereo rig;
        if(argc==2&&std::string_view(argv[1])=="inspect")inspect(rig,{1,1,-4},0);
        else if(argc==2&&std::string_view(argv[1])=="repl")repl();
        else if(argc==3&&(std::string_view(argv[1])=="svg"||std::string_view(argv[1])=="csv"))
            save(rig,argv[2],std::string_view(argv[1])=="svg");
        else{std::cerr<<"book03_stereo inspect|repl|csv FILE|svg FILE\n";return 2;}
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
