// P14 projection debugger: real scalar clip/NDC/viewport values and a
// controlled depth STORAGE quantization study. No GPU timing or rasterizer.
#include "book03/projection_pipeline.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
namespace {
using namespace book03;
void inspect(const ProjectionMatrix& m,Point3 p) {
    const auto t=trace_projection(m,{},p);
    std::cout<<std::setprecision(17)<<"clip "<<t.clip.x<<","<<t.clip.y<<","
        <<t.clip.z<<","<<t.clip.w<<"\nplanes ";
    for(bool b:t.classification.pass)std::cout<<(b?'1':'0');
    std::cout<<" positive-w="<<t.classification.positive_w<<"\n";
    if(t.screen)std::cout<<"ndc "<<t.ndc->x<<","<<t.ndc->y<<","<<t.ndc->z
        <<"\nscreen "<<t.screen->image.u<<","<<t.screen->image.v
        <<" depth="<<t.screen->depth<<"\n";
    else std::cout<<"Rejected by clip membership\n";
}
// Quantize the analytically evaluated depth to binary32 or a uniformly
// spaced 24-bit fixed-point grid. Decode with the SAME mathematical map.
// This isolates storage rounding; it is NOT a prediction of GPU arithmetic,
// actual depth-buffer format behavior, Z fighting, or complete error bounds.
double quantize(double q,bool floating) {
    q=std::clamp(q,0.0,1.0); // controlled in-range analytic samples only
    constexpr double levels=16777215.0;
    return floating?static_cast<double>(static_cast<float>(q)):
        std::round(q*levels)/levels;
}
void depth_csv(std::ostream& out){
    out<<"axial,order,storage,ideal,stored,reconstructed,abs_error,status\n"
        <<std::setprecision(17);
    for(int i=0;i<=240;++i){
        const double d=.1*std::pow(10.0,i/40.0);
        for(auto order:{DepthOrder::Forward,DepthOrder::Reverse}){
            const DepthMap map{.1,std::nullopt,order};
            // Analytic infinite maps isolate storage precision from clip
            // matrix arithmetic. Exact forward formula rounds in binary64.
            const double q=map.perspective_depth(d);
            for(bool fp:{false,true}){
                const double stored=quantize(q,fp);
                const auto reconstructed=map.inverse_perspective(stored);
                out<<d<<","<<(order==DepthOrder::Forward?"forward":"reverse")<<","
                   <<(fp?"float32":"unorm24")<<","<<q<<","<<stored<<",";
                if(reconstructed)out<<*reconstructed<<","<<std::abs(*reconstructed-d)<<",finite\n";
                else out<<",,infinity\n";
            }
        }
    }
}
void depth_svg(std::ostream& out) {
    // Plot q itself, not a claimed hardware precision or image quality score.
    // Log-spaced axial distances, infinite far, n=.1.
    const DepthMap f{.1,std::nullopt,DepthOrder::Forward},
                   r{.1,std::nullopt,DepthOrder::Reverse};
    out<<"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 1080 560\" role=\"img\">"
        "<title>Reciprocal depth mapping</title>"
        "<desc>Actual scalar infinite perspective depth values for near 0.1, forward and reverse, versus logarithmic axial distance.</desc>"
        "<rect width=\"1080\" height=\"560\" fill=\"#fbfaf7\"/>"
        "<text x=\"28\" y=\"40\" font-size=\"24\" font-family=\"sans-serif\">Infinite perspective depth: forward and reversed</text>";
    for(int i=0;i<=6;++i){
        const double x=90+150*i;
        out<<"<line x1=\""<<x<<"\" x2=\""<<x<<"\" y1=\"85\" y2=\"450\" stroke=\"#d5e1da\"/>"
           <<"<text x=\""<<x-15<<"\" y=\"475\" font-size=\"13\">"<<.1*std::pow(10.,i)<<"</text>";
    }
    for(int i=0;i<=4;++i){
        const double y=450-365*i/4.;
        out<<"<line x1=\"90\" x2=\"990\" y1=\""<<y<<"\" y2=\""<<y<<"\" stroke=\"#d5e1da\"/>"
           <<"<text x=\"42\" y=\""<<y+4<<"\" font-size=\"13\">"<<i/4.0<<"</text>";
    }
    for(const auto& map:{f,r}){
        out<<"<polyline fill=\"none\" stroke=\""
           <<(map.order==DepthOrder::Forward?"#b37850":"#347d6d")
           <<"\" stroke-width=\"3\" points=\"";
        for(int i=0;i<=240;++i){
            const double d=.1*std::pow(10.,i/40.);
            out<<90+900*i/240.<<","<<450-365*map.perspective_depth(d)<<" ";
        }
        out<<"\"/>";
    }
    out<<"<text x=\"90\" y=\"515\" font-size=\"14\" font-family=\"sans-serif\">Axial distance (m), logarithmic axis. Forward: brown; reverse: green. Near = 0.1 m.</text></svg>\n";
}
void save(const std::string& p,bool picture){
    std::ofstream out(p);if(!out)throw std::runtime_error("cannot open output");
    if(picture)depth_svg(out);else depth_csv(out);
    out.flush();if(!out)throw std::runtime_error("output failed");
}
void repl(){
    DepthMap depth{1,9,DepthOrder::Forward};bool ortho=false;
    Point3 p{1,1,-4};
    std::cout<<"point X Y Z | depth NEAR FAR (0=infinite) | reverse | forward | perspective | ortho | inspect | csv FILE | svg FILE | quit\n";
    std::string line;
    while(std::getline(std::cin,line)){
        std::istringstream in(line);std::string cmd;in>>cmd;
        try{
            if(cmd=="quit")break;
            if(cmd=="point"){
                Point3 q;if(!(in>>q.x>>q.y>>q.z)||!finite(q))throw std::invalid_argument("point syntax");
                p=q;
            }else if(cmd=="depth"){
                double n,f;if(!(in>>n>>f))throw std::invalid_argument("depth syntax");
                DepthMap d{n,f==0?std::nullopt:std::optional<double>{f},depth.order};
                d.validate();if(ortho)(void)orthographic_matrix({},d);
                else (void)perspective_matrix({},d);depth=d;
            }else if(cmd=="reverse")depth.order=DepthOrder::Reverse;
            else if(cmd=="forward")depth.order=DepthOrder::Forward;
            else if(cmd=="perspective")ortho=false;
            else if(cmd=="ortho"){(void)orthographic_matrix({},depth);ortho=true;}
            else if(cmd=="inspect")inspect(ortho?orthographic_matrix({},depth):
                                            perspective_matrix({},depth),p);
            else if(cmd=="csv"||cmd=="svg"){
                std::string path;if(!(in>>path))throw std::invalid_argument("missing output");
                save(path,cmd=="svg"); // always the fixed reference depth study
            }else if(!cmd.empty())std::cout<<"Unknown command\n";
        }catch(const std::exception& e){std::cout<<"Rejected: "<<e.what()<<"\n";}
    }
}
}
int main(int argc,char** argv){
    try{
        if(argc==2&&std::string_view(argv[1])=="inspect")
            inspect(perspective_matrix({}, {1,9}),{1,1,-4});
        else if(argc==2&&std::string_view(argv[1])=="repl")repl();
        else if(argc==3&&(std::string_view(argv[1])=="csv"||std::string_view(argv[1])=="svg"))
            save(argv[2],std::string_view(argv[1])=="svg");
        else{std::cerr<<"book03_projection inspect|repl|csv FILE|svg FILE\n";return 2;}
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
