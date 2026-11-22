// PROJECTALLMIGHT GGUF v3 subset parser based on the current ggml GGUF structure.
// Covers typed metadata, tensor descriptors, alignment and bounds; deliberately not every metadata type or GGML tensor encoding.
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

template<class U>
U read_le(const std::vector<std::uint8_t>& b,std::size_t& c){
    U v=0; require(c+sizeof(U)<=b.size(),"truncated integer");
    for(std::size_t i=0;i<sizeof(U);++i) v|=static_cast<U>(b[c+i])<<(8*i);
    c+=sizeof(U); return v;
}
template<class U>
void put_le(std::vector<std::uint8_t>& b,U v){ for(std::size_t i=0;i<sizeof(U);++i)b.push_back(static_cast<std::uint8_t>((v>>(8*i))&0xffu)); }

void put_string(std::vector<std::uint8_t>& b,const std::string& s){ put_le<std::uint64_t>(b,s.size()); b.insert(b.end(),s.begin(),s.end()); }
std::string read_string(const std::vector<std::uint8_t>& b,std::size_t& c){
    const auto n=read_le<std::uint64_t>(b,c); require(n<=b.size()-c,"truncated string"); std::string s(reinterpret_cast<const char*>(b.data()+c),static_cast<std::size_t>(n)); c+=static_cast<std::size_t>(n); return s;
}

enum class GgufType:std::uint32_t{ UInt32=4,String=8 };
struct Meta{ std::string key; GgufType type; std::uint32_t u32{}; std::string str; };
struct TensorInfo{ std::string name; std::vector<std::uint64_t> dims; std::uint32_t type{}; std::uint64_t offset{}; };
struct Parsed{ std::uint32_t version{}; std::uint32_t alignment=32; std::vector<Meta> meta; std::vector<TensorInfo> tensors; std::size_t tensor_data_start{}; };

std::size_t align_up(std::size_t x,std::size_t a){ require(a>0,"zero alignment"); return x+(a-x%a)%a; }

Parsed parse(const std::vector<std::uint8_t>& b){
    require(b.size()>=4&&std::memcmp(b.data(),"GGUF",4)==0,"GGUF magic mismatch");
    std::size_t c=4; Parsed out;
    out.version=read_le<std::uint32_t>(b,c);
    const auto tensor_count=read_le<std::uint64_t>(b,c);
    const auto meta_count=read_le<std::uint64_t>(b,c);
    require(out.version==3,"teaching parser accepts GGUF v3 only");
    require(tensor_count<1024&&meta_count<1024,"teaching file declares unreasonable counts");

    for(std::uint64_t i=0;i<meta_count;++i){
        Meta m; m.key=read_string(b,c); require(!m.key.empty(),"empty metadata key");
        m.type=static_cast<GgufType>(read_le<std::uint32_t>(b,c));
        if(m.type==GgufType::UInt32) m.u32=read_le<std::uint32_t>(b,c);
        else if(m.type==GgufType::String) m.str=read_string(b,c);
        else fail("unsupported metadata type in subset parser");
        if(m.key=="general.alignment"){
            require(m.type==GgufType::UInt32,"general.alignment must be UINT32");
            require(m.u32>=8&&m.u32%8==0,"GGUF alignment must be a positive multiple of 8 in this witness");
            out.alignment=m.u32;
        }
        out.meta.push_back(std::move(m));
    }

    for(std::uint64_t i=0;i<tensor_count;++i){
        TensorInfo t; t.name=read_string(b,c); require(t.name.size()<=64,"tensor name exceeds GGUF limit");
        const auto nd=read_le<std::uint32_t>(b,c); require(nd>0&&nd<=4,"tensor rank outside current teaching bound");
        for(std::uint32_t d=0;d<nd;++d){ const auto dim=read_le<std::uint64_t>(b,c); require(dim>0,"zero tensor dimension"); t.dims.push_back(dim); }
        t.type=read_le<std::uint32_t>(b,c);
        t.offset=read_le<std::uint64_t>(b,c);
        require(t.offset%out.alignment==0,"tensor offset violates GGUF alignment");
        out.tensors.push_back(std::move(t));
    }
    out.tensor_data_start=align_up(c,out.alignment);
    require(out.tensor_data_start<=b.size(),"tensor-data alignment runs beyond file");
    return out;
}

std::vector<std::uint8_t> make_file(std::uint32_t alignment){
    std::vector<std::uint8_t> b{'G','G','U','F'};
    put_le<std::uint32_t>(b,3); put_le<std::uint64_t>(b,1); put_le<std::uint64_t>(b,2);
    put_string(b,"general.alignment"); put_le<std::uint32_t>(b,4); put_le<std::uint32_t>(b,alignment);
    put_string(b,"general.architecture"); put_le<std::uint32_t>(b,8); put_string(b,"llama");
    put_string(b,"blk.0.weight"); put_le<std::uint32_t>(b,2); put_le<std::uint64_t>(b,4); put_le<std::uint64_t>(b,8);
    put_le<std::uint32_t>(b,0); put_le<std::uint64_t>(b,0);
    while(b.size()%alignment) b.push_back(0);
    b.resize(b.size()+4*8*sizeof(float),0);
    return b;
}

void test_valid(){
    const auto bytes=make_file(32);
    const Parsed p=parse(bytes);
    require(p.version==3&&p.alignment==32,"header/alignment parse wrong");
    require(p.meta.size()==2&&p.tensors.size()==1,"GGUF record counts wrong");
    require(p.meta[1].str=="llama","architecture metadata wrong");
    require(p.tensors[0].name=="blk.0.weight"&&p.tensors[0].dims==std::vector<std::uint64_t>({4,8}),"tensor descriptor wrong");
    require(p.tensor_data_start%32==0,"tensor data start not aligned");
}

void test_rejections(){
    bool bad_align=false;
    try{ (void)parse(make_file(12)); }catch(const std::runtime_error&){ bad_align=true; }
    require(bad_align,"invalid alignment must be rejected");

    auto truncated=make_file(32); truncated.resize(20);
    bool bad_trunc=false;
    try{ (void)parse(truncated); }catch(const std::runtime_error&){ bad_trunc=true; }
    require(bad_trunc,"truncated GGUF must be rejected");
}

} // namespace

int main(){
    try{
        test_valid(); test_rejections();
        std::cout<<"gguf_metadata_tensor_lab: PASS\n"
                 <<"proved: GGUF v3 magic/counts, typed UINT32+STRING metadata, general.alignment, tensor rank/dimensions/type/relative offset, aligned tensor-data boundary, truncation rejection\n"
                 <<"not proved: all GGUF metadata/array types, big-endian files, every ggml quant type/block size, mmap paging, tokenizer metadata, tensor payload validation, or llama.cpp parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"gguf_metadata_tensor_lab: FAIL: "<<e.what()<<'\n'; return 1;
    }
}
