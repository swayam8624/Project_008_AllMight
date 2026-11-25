#include "book03/studio.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
using namespace book03;
using namespace book03::studio;
int main(int argc,char** argv){
    if(argc!=2){std::cerr<<"usage: book03_studio NEW_OUTPUT_DIRECTORY\n";return 2;}
    try{
        const auto truth=house();const auto rec=reconstruct(truth);auto original=render(truth),recovered=render(rec.mesh);
        const std::filesystem::path dir(argv[1]);if(!std::filesystem::create_directory(dir))throw std::runtime_error("output directory must be new");
        std::ofstream a(dir/"reference.ppm",std::ios::binary),b(dir/"reconstructed.ppm",std::ios::binary),c(dir/"reconstructed.obj"),d(dir/"tracks.csv"),e(dir/"metrics.csv");
        original.ppm(a);recovered.ppm(b);obj(c,rec.mesh);
        d<<std::setprecision(17)<<"landmark,camera,u,v\n";for(std::size_t i=0;i<rec.tracks.size();++i)for(auto o:rec.tracks[i])d<<i<<','<<o.camera<<','<<o.pixel[0]<<','<<o.pixel[1]<<'\n';
        e<<std::setprecision(17)<<"vertices,faces,observations,max_3d_m,rms_px\n"<<rec.mesh.points.size()<<','<<rec.mesh.faces.size()<<','<<rec.mesh.points.size()*cameras().size()<<','<<rec.max_3d<<','<<rec.rms_pixels<<'\n';
        for(auto* file:{&a,&b,&c,&d,&e}){file->close();if(!*file)throw std::runtime_error("file write failed");}
        std::cout<<"P31 studio completed: "<<dir<<'\n';return 0;
    }catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}
}
