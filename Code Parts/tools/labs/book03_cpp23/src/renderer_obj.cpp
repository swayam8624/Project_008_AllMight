// Front-end integration executable; reads the strict P10 OBJ subset.
// Input stays in object coordinates. The chosen transform is a simple
// translation into view; arbitrary assets may require other placement.
#include "book03/renderer_frontend.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
using namespace book03;
int main(int argc,char** argv){try{
    if(argc!=4||std::string_view(argv[1])!="obj")
        throw std::invalid_argument("book03_renderer obj INPUT.obj OUTPUT.ppm");
    if(std::filesystem::file_size(argv[2])>5*1024*1024)
        throw std::invalid_argument("OBJ exceeds 5 MiB");
    if(std::filesystem::exists(argv[3])&&std::filesystem::equivalent(argv[2],argv[3]))
        throw std::invalid_argument("input and output must be different");
    std::ifstream in(argv[2]);if(!in)throw std::runtime_error("OBJ open failed");
    auto mesh=import_obj(in).to_rendering_mesh();
    SoftwareRenderer r(640,480);
    const auto stats=draw_mesh(r,mesh,scene_translate({0,0,-4}),{},
        perspective_matrix({}, {1,20}),{},1,{.34,.67,.5});
    std::ofstream out(argv[3],std::ios::binary);if(!out)throw std::runtime_error("output open failed");
    r.ppm(out);out.flush();if(!out)throw std::runtime_error("output write failed");
    std::cout<<"vertices="<<stats.vertices<<" input="<<stats.input_triangles
        <<" rejected="<<stats.clipped_away<<" fan="<<stats.fan_triangles
        <<" covered="<<r.stats().covered<<"\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
