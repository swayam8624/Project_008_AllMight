// PROJECTALLMIGHT POSIX atomic-publication durability witness.
// Executes write -> fsync(file) -> rename -> fsync(directory) and validates publication state.
// Successful fsync calls are evidence of requested durability ordering, not a simulated power-loss proof.
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

void checked_close(int fd,const char* what){
    if(::close(fd)!=0) fail(std::string(what)+": "+std::strerror(errno));
}
void write_all(int fd,const std::string& data){
    std::size_t off=0;
    while(off<data.size()){
        const ssize_t n=::write(fd,data.data()+off,data.size()-off);
        if(n<0){
            if(errno==EINTR) continue;
            fail(std::string("write: ")+std::strerror(errno));
        }
        require(n>0,"write made no progress");
        off+=static_cast<std::size_t>(n);
    }
}

struct TempDir{
    std::filesystem::path path;
    TempDir(){
        std::string pattern=(std::filesystem::temp_directory_path()/"allmight-publish-XXXXXX").string();
        std::vector<char> chars(pattern.begin(),pattern.end());
        chars.push_back('\0');
        char* made=::mkdtemp(chars.data());
        if(!made) fail(std::string("mkdtemp: ")+std::strerror(errno));
        path=made;
    }
    ~TempDir(){ std::error_code ec; std::filesystem::remove_all(path,ec); }
};

void publish(const std::filesystem::path& dir,const std::string& final_name,const std::string& payload){
    require(final_name.find('/')==std::string::npos&&final_name!="."&&final_name!="..",
            "teaching publisher accepts one validated leaf name only");

    const auto tmp=dir/(final_name+".tmp");
    const auto final=dir/final_name;

    const int fd=::open(tmp.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0644);
    if(fd<0) fail(std::string("open temp: ")+std::strerror(errno));
    try{
        write_all(fd,payload);
        if(::fsync(fd)!=0) fail(std::string("fsync file: ")+std::strerror(errno));
        checked_close(fd,"close temp");
    }catch(...){
        ::close(fd);
        ::unlink(tmp.c_str());
        throw;
    }

    if(::rename(tmp.c_str(),final.c_str())!=0)
        fail(std::string("rename: ")+std::strerror(errno));

    const int dfd=::open(dir.c_str(),O_RDONLY|O_DIRECTORY|O_CLOEXEC);
    if(dfd<0) fail(std::string("open directory: ")+std::strerror(errno));
    if(::fsync(dfd)!=0){
        const int saved=errno;
        ::close(dfd);
        fail(std::string("fsync directory: ")+std::strerror(saved));
    }
    checked_close(dfd,"close directory");
}

std::string read_file(const std::filesystem::path& p){
    std::ifstream in(p,std::ios::binary);
    require(static_cast<bool>(in),"open final for verification failed");
    return {std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>()};
}

void test_publication(){
    TempDir td;
    const std::string payload="PROJECTALLMIGHT\nmanifest=42\nchecksum=demo\n";
    publish(td.path,"asset.pkg",payload);
    require(read_file(td.path/"asset.pkg")==payload,"published bytes changed");
    require(!std::filesystem::exists(td.path/"asset.pkg.tmp"),"temporary publication name leaked");
}

void test_no_overwrite_and_leaf_validation(){
    TempDir td;
    publish(td.path,"stable.bin","v1");

    // O_EXCL protects the temporary writer from colliding with a pre-existing temp name.
    {
        std::ofstream(td.path/"other.bin.tmp")<<"attacker-or-stale-temp";
        bool rejected=false;
        try{ publish(td.path,"other.bin","v2"); }catch(const std::runtime_error&){ rejected=true; }
        require(rejected,"existing staging file must reject O_EXCL publication");
        require(!std::filesystem::exists(td.path/"other.bin"),"failed publication created final name");
    }

    bool traversal=false;
    try{ publish(td.path,"../escape","bad"); }catch(const std::runtime_error&){ traversal=true; }
    require(traversal,"publisher accepted path traversal instead of leaf name");
}

} // namespace

int main(){
    try{
        test_publication();
        test_no_overwrite_and_leaf_validation();
        std::cout
            <<"atomic_publication_durability_lab: PASS\n"
            <<"proved: real POSIX exclusive staging file, complete writes, fsync(file), atomic same-directory rename, fsync(directory), final-byte verification, stale-temp and traversal rejection\n"
            <<"not proved: actual power-cut survival on every filesystem/device, Windows durability semantics, replacement/version policy, multi-file transactions, storage-controller caches, or production asset-pipeline parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"atomic_publication_durability_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
