// P11: source-derived instance placement, graph editing and CSV/SVG exports.
// This is an oblique diagram, not a perspective renderer. The mesh is shared
// until the diagnostic writer projects each instance's transformed positions.
#include "book03/scene_forest.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
namespace {
using namespace book03;
void inspect(const SceneForest& s) {
    const auto e=s.evaluate();
    std::cout<<"Assets="<<s.assets.size()<<" Nodes="<<s.nodes.size()
             <<" Instances="<<e.instances.size()<<'\n';
    for(const auto i:e.order) {
        const auto p=e.world[i].point({0,0,0});
        std::cout<<"node "<<i<<" "<<s.nodes[i].name<<" origin=("
                 <<p.x<<","<<p.y<<","<<p.z<<") determinant="
                 <<e.world[i].linear.determinant()<<'\n';
    }
}
void csv(std::ostream& out,const SceneForest& s) {
    out<<"node,asset,x,y,z,min_x,min_y,min_z,max_x,max_y,max_z,determinant\n";
    out<<std::setprecision(17);
    for(const auto& i:s.evaluate().instances) {
        const auto p=i.world.point({0,0,0});
        const auto a=i.bounds.min,b=i.bounds.max;
        out<<i.node<<","<<i.asset<<","<<p.x<<","<<p.y<<","<<p.z<<","
           <<a.x<<","<<a.y<<","<<a.z<<","<<b.x<<","<<b.y<<","<<b.z<<","
           <<i.world.linear.determinant()<<'\n';
    }
}
void svg(std::ostream& out,const SceneForest& s) {
    const auto e=s.evaluate();
    // Fit the projected bounds to the drawing, independent of scene size.
    const auto project=[](Point3 p){return std::array<double,2>{
        p.x-0.55*p.z,-p.y+0.24*p.z};};
    double xmin=std::numeric_limits<double>::infinity(),ymin=xmin,
           xmax=-xmin,ymax=-xmin;
    for(const auto& i:e.instances)for(const auto p:s.assets[i.asset].positions) {
        const auto q=project(i.world.point(p));
        xmin=std::min(xmin,q[0]);xmax=std::max(xmax,q[0]);
        ymin=std::min(ymin,q[1]);ymax=std::max(ymax,q[1]);
    }
    const double scale=std::min(970/std::max(1.0,xmax-xmin),
                               380/std::max(1.0,ymax-ymin));
    const auto page=[&](Point3 p){auto q=project(p);
        return std::array<double,2>{65+(q[0]-xmin)*scale,
                                    125+(q[1]-ymin)*scale};};
    out<<"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 1100 590\" role=\"img\">"
        "<title>Five transformed instances of one tree asset</title>"
        "<desc>Oblique projection of actual shared mesh positions after scene graph evaluation. No visibility or perspective calculation.</desc>"
        "<rect width=\"1100\" height=\"590\" fill=\"#fbfaf6\"/>"
        "<text x=\"30\" y=\"38\" font-family=\"sans-serif\" font-size=\"24\" fill=\"#284e58\">One tree asset; five world transforms</text>";
    for(const auto& i:e.instances) {
        const auto& mesh=s.assets[i.asset];
        for(const auto& f:mesh.triangles) {
            out<<"<polygon points=\"";
            for(auto id:f.vertex) {
                auto q=page(i.world.point(mesh.positions[id]));
                out<<q[0]<<","<<q[1]<<" ";
            }
            out<<"\" fill=\"#73aa91\" stroke=\"#376c61\" stroke-width=\"1.5\"/>";
        }
        const auto q=page(i.world.point({0,0,0}));
        out<<"<text x=\""<<q[0]<<"\" y=\""<<q[1]+18
           <<"\" font-family=\"sans-serif\" font-size=\"12\">node "<<i.node<<"</text>";
    }
    out<<"<text x=\"30\" y=\"565\" font-family=\"sans-serif\" font-size=\"14\" fill=\"#586f75\">Shared mesh storage; per-instance transforms. Illustrative parallel projection.</text></svg>\n";
}
void save(const SceneForest& s,const std::string& name,bool picture) {
    std::ofstream out(name);
    if(!out)throw std::runtime_error("cannot open output");
    if(picture)svg(out,s);else csv(out,s);
    if(!out)throw std::runtime_error("export failed");
}
void repl(SceneForest s) {
    std::cout<<"inspect | parent CHILD PARENT|-1 local|world | move NODE X Y Z | svg FILE | csv FILE | quit\n";
    std::string line;
    while(std::getline(std::cin,line)) {
        std::istringstream in(line);std::string cmd;in>>cmd;
        try {
            if(cmd=="quit")break;
            if(cmd=="inspect")inspect(s);
            else if(cmd=="parent") {
                std::size_t c;long long p;std::string mode;
                if(!(in>>c>>p>>mode)||p< -1||(mode!="world"&&mode!="local"))
                    throw std::invalid_argument("parent syntax");
                s.reparent(c,p<0?std::nullopt:std::optional<std::size_t>{
                    static_cast<std::size_t>(p)},mode=="world");
                std::cout<<"Updated parent\n";
            } else if(cmd=="move") {
                std::size_t id;Vector3 v;
                if(!(in>>id>>v.x>>v.y>>v.z)||id>=s.nodes.size()||!finite(v))
                    throw std::invalid_argument("move syntax");
                auto candidate=s;candidate.nodes[id].local.translation=v;
                (void)candidate.evaluate();s=std::move(candidate);
                std::cout<<"Updated translation\n";
            } else if(cmd=="svg"||cmd=="csv") {
                std::string path;if(!(in>>path))throw std::invalid_argument("missing path");
                save(s,path,cmd=="svg");
            } else if(!cmd.empty())std::cout<<"Unknown command\n";
        }catch(const std::exception& e){std::cout<<"Rejected: "<<e.what()<<'\n';}
    }
}
}
int main(int argc,char** argv) {
    try {
        auto s=five_tree_world();
        if(argc==2&&std::string_view(argv[1])=="inspect")inspect(s);
        else if(argc==2&&std::string_view(argv[1])=="repl")repl(std::move(s));
        else if(argc==3&&(std::string_view(argv[1])=="svg"||
                          std::string_view(argv[1])=="csv"))
            save(s,argv[2],std::string_view(argv[1])=="svg");
        else {std::cerr<<"book03_scene inspect|repl|svg FILE|csv FILE\n";return 2;}
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
