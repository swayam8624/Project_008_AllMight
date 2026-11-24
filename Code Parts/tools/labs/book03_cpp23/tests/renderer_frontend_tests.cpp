#include "book03/renderer_frontend.hpp"
#include <sstream>
#include <iostream>
using namespace book03;
int main(){try{
    // A calibrated reference quad lies at depth four. Its projected
    // corners are (220,340),(420,340),(420,140),(220,140).
    std::istringstream in("v -1 -1 0\nv 1 -1 0\nv 1 1 0\nv -1 1 0\n"
        "vt 0 0\nvt 1 0\nvt 1 1\nvt 0 1\nf 1/1 2/2 3/3\nf 1/1 3/3 4/4\n");
    auto mesh=import_obj(in).to_rendering_mesh();SoftwareRenderer r(640,480);
    auto s=draw_mesh(r,mesh,scene_translate({0,0,-4}),{},
        perspective_matrix({}, {1,20}),{},7,{1,.5,.25});
    if(s.vertices!=4||s.input_triangles!=2||s.fan_triangles!=2||r.stats().covered!=40000)
        throw std::runtime_error("front-end reference quad");
    for(int y=0;y<480;++y)for(int x=0;x<640;++x){
        const auto p=r.winner(x,y);
        if(bool(p)!=(x>=220&&x<420&&y>=140&&y<340)||(p&&p->id!=7))
            throw std::runtime_error("front-end ownership");
    }
    std::cout<<"Book03/P21 front-end contracts: PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
