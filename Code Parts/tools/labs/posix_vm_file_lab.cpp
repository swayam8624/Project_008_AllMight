// PROJECTALLMIGHT Phase 28 native POSIX witness: descriptor lifetime, shared file offset, mmap and persistence.
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

struct Fd{
    int v=-1;
    explicit Fd(int x=-1):v(x){}
    Fd(const Fd&)=delete; Fd& operator=(const Fd&)=delete;
    Fd(Fd&& o) noexcept:v(std::exchange(o.v,-1)){}
    ~Fd(){ if(v>=0) ::close(v); }
};

void write_all(int fd,const char* p,std::size_t n){
    while(n){
        const ssize_t w=::write(fd,p,n);
        if(w<0) fail("write failed");
        require(w>0,"write made no progress");
        p+=w;n-=static_cast<std::size_t>(w);
    }
}

void test_dup_shares_open_file_offset(){
    char name[]="/tmp/allmight-posix-XXXXXX";
    Fd fd(::mkstemp(name));
    require(fd.v>=0,"mkstemp failed");
    ::unlink(name); // anonymous lifetime after open descriptors close.

    write_all(fd.v,"abcdef",6);
    require(::lseek(fd.v,0,SEEK_SET)==0,"lseek failed");

    Fd dupfd(::dup(fd.v));
    require(dupfd.v>=0,"dup failed");

    char a[3]{};
    require(::read(fd.v,a,2)==2&&std::string(a,2)=="ab","first read mismatch");
    char b[3]{};
    require(::read(dupfd.v,b,2)==2&&std::string(b,2)=="cd",
            "dup descriptors should share the open-file-description offset");
}

void test_file_backed_mmap(){
    char name[]="/tmp/allmight-mmap-XXXXXX";
    Fd fd(::mkstemp(name));
    require(fd.v>=0,"mkstemp failed");
    ::unlink(name);
    constexpr std::size_t size=4096;
    require(::ftruncate(fd.v,static_cast<off_t>(size))==0,"ftruncate failed");

    void* p=::mmap(nullptr,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd.v,0);
    require(p!=MAP_FAILED,"mmap failed");
    auto* bytes=static_cast<unsigned char*>(p);
    bytes[0]=0x12; bytes[123]=0x34; bytes[size-1]=0x56;
    require(::msync(p,size,MS_SYNC)==0,"msync failed");
    require(::munmap(p,size)==0,"munmap failed");

    std::array<unsigned char,3> check{};
    require(::pread(fd.v,&check[0],1,0)==1,"pread 0 failed");
    require(::pread(fd.v,&check[1],1,123)==1,"pread 123 failed");
    require(::pread(fd.v,&check[2],1,static_cast<off_t>(size-1))==1,"pread last failed");
    require(check[0]==0x12&&check[1]==0x34&&check[2]==0x56,"MAP_SHARED changes not visible through file reads");
}

} // namespace

int main(){
    try{
        test_dup_shares_open_file_offset();
        test_file_backed_mmap();
        std::cout<<"posix_vm_file_lab: PASS\n"
                 <<"proved: dup shares an open-file-description offset, unlinked-open-file lifetime, file-backed MAP_SHARED write/read visibility after msync\n"
                 <<"not proved: crash durability, filesystem atomicity, anonymous paging/TLB behavior, Windows APIs, page-fault performance, or production asset-loader semantics\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"posix_vm_file_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
