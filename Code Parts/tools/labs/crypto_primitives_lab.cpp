// PROJECTALLMIGHT Phase 28 educational cryptographic-primitives witness.
// Known-answer tests make the bit-level implementation inspectable.
// Production systems should use vetted cryptographic libraries and key-management protocols.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

using Digest=std::array<std::uint8_t,32>;

constexpr std::array<std::uint32_t,64> K{
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

std::uint32_t rotr(std::uint32_t x,int n){ return (x>>n)|(x<<(32-n)); }

Digest sha256(std::vector<std::uint8_t> msg){
    const std::uint64_t bit_length=static_cast<std::uint64_t>(msg.size())*8ULL;
    msg.push_back(0x80);
    while((msg.size()%64)!=56) msg.push_back(0);
    for(int i=7;i>=0;--i) msg.push_back(static_cast<std::uint8_t>((bit_length>>(8*i))&0xffu));

    std::array<std::uint32_t,8> h{
        0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,
        0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u};

    for(std::size_t block=0;block<msg.size();block+=64){
        std::array<std::uint32_t,64> w{};
        for(int i=0;i<16;++i){
            const std::size_t o=block+4*static_cast<std::size_t>(i);
            w[i]=(static_cast<std::uint32_t>(msg[o])<<24)|
                 (static_cast<std::uint32_t>(msg[o+1])<<16)|
                 (static_cast<std::uint32_t>(msg[o+2])<<8)|
                  static_cast<std::uint32_t>(msg[o+3]);
        }
        for(int i=16;i<64;++i){
            const std::uint32_t s0=rotr(w[i-15],7)^rotr(w[i-15],18)^(w[i-15]>>3);
            const std::uint32_t s1=rotr(w[i-2],17)^rotr(w[i-2],19)^(w[i-2]>>10);
            w[i]=w[i-16]+s0+w[i-7]+s1;
        }

        auto [a,b,c,d,e,f,g,hh]=h;
        for(int i=0;i<64;++i){
            const std::uint32_t S1=rotr(e,6)^rotr(e,11)^rotr(e,25);
            const std::uint32_t ch=(e&f)^((~e)&g);
            const std::uint32_t temp1=hh+S1+ch+K[i]+w[i];
            const std::uint32_t S0=rotr(a,2)^rotr(a,13)^rotr(a,22);
            const std::uint32_t maj=(a&b)^(a&c)^(b&c);
            const std::uint32_t temp2=S0+maj;
            hh=g; g=f; f=e; e=d+temp1;
            d=c; c=b; b=a; a=temp1+temp2;
        }
        h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=hh;
    }

    Digest out{};
    for(std::size_t i=0;i<8;++i){
        out[4*i]=static_cast<std::uint8_t>(h[i]>>24);
        out[4*i+1]=static_cast<std::uint8_t>(h[i]>>16);
        out[4*i+2]=static_cast<std::uint8_t>(h[i]>>8);
        out[4*i+3]=static_cast<std::uint8_t>(h[i]);
    }
    return out;
}

std::vector<std::uint8_t> bytes(std::string_view s){
    return {s.begin(),s.end()};
}

std::string hex(const std::vector<std::uint8_t>& v){
    std::ostringstream out;
    out<<std::hex<<std::setfill('0');
    for(std::uint8_t b:v) out<<std::setw(2)<<static_cast<int>(b);
    return out.str();
}
std::string hex(const Digest& d){ return hex(std::vector<std::uint8_t>(d.begin(),d.end())); }

std::vector<std::uint8_t> from_hex(const std::string& s){
    require(s.size()%2==0,"hex length must be even");
    std::vector<std::uint8_t> out;
    out.reserve(s.size()/2);
    auto nibble=[](char c)->int{
        if(c>='0'&&c<='9') return c-'0';
        if(c>='a'&&c<='f') return 10+c-'a';
        if(c>='A'&&c<='F') return 10+c-'A';
        fail("invalid hex digit");
    };
    for(std::size_t i=0;i<s.size();i+=2) out.push_back(static_cast<std::uint8_t>((nibble(s[i])<<4)|nibble(s[i+1])));
    return out;
}

Digest hmac_sha256(std::vector<std::uint8_t> key,const std::vector<std::uint8_t>& data){
    constexpr std::size_t block=64;
    if(key.size()>block){
        const Digest h=sha256(key);
        key.assign(h.begin(),h.end());
    }
    key.resize(block,0);

    std::vector<std::uint8_t> inner(block),outer(block);
    for(std::size_t i=0;i<block;++i){
        inner[i]=static_cast<std::uint8_t>(key[i]^0x36u);
        outer[i]=static_cast<std::uint8_t>(key[i]^0x5cu);
    }
    inner.insert(inner.end(),data.begin(),data.end());
    const Digest ih=sha256(std::move(inner));
    outer.insert(outer.end(),ih.begin(),ih.end());
    return sha256(std::move(outer));
}

Digest hkdf_extract(const std::vector<std::uint8_t>& salt,const std::vector<std::uint8_t>& ikm){
    std::vector<std::uint8_t> effective=salt;
    if(effective.empty()) effective.assign(32,0);
    return hmac_sha256(std::move(effective),ikm);
}

std::vector<std::uint8_t> hkdf_expand(
    const Digest& prk,const std::vector<std::uint8_t>& info,std::size_t length){
    require(length<=255u*32u,"HKDF output too long");
    std::vector<std::uint8_t> okm;
    std::vector<std::uint8_t> previous;
    for(std::uint16_t counter=1;okm.size()<length;++counter){
        std::vector<std::uint8_t> input=previous;
        input.insert(input.end(),info.begin(),info.end());
        input.push_back(static_cast<std::uint8_t>(counter));
        const Digest block=hmac_sha256(std::vector<std::uint8_t>(prk.begin(),prk.end()),input);
        previous.assign(block.begin(),block.end());
        const std::size_t take=std::min<std::size_t>(previous.size(),length-okm.size());
        okm.insert(okm.end(),previous.begin(),previous.begin()+static_cast<std::ptrdiff_t>(take));
    }
    return okm;
}

Digest merkle_leaf(const std::vector<std::uint8_t>& data){
    std::vector<std::uint8_t> tagged;
    tagged.reserve(data.size()+1);
    tagged.push_back(0x00);
    tagged.insert(tagged.end(),data.begin(),data.end());
    return sha256(std::move(tagged));
}
Digest merkle_parent(const Digest& a,const Digest& b){
    std::vector<std::uint8_t> tagged;
    tagged.reserve(65);
    tagged.push_back(0x01);
    tagged.insert(tagged.end(),a.begin(),a.end());
    tagged.insert(tagged.end(),b.begin(),b.end());
    return sha256(std::move(tagged));
}
Digest merkle_root(const std::vector<std::vector<std::uint8_t>>& leaves){
    require(!leaves.empty(),"Merkle tree needs at least one leaf");
    std::vector<Digest> level;
    for(const auto& leaf:leaves) level.push_back(merkle_leaf(leaf));
    while(level.size()>1){
        if(level.size()%2==1) level.push_back(level.back());
        std::vector<Digest> next;
        for(std::size_t i=0;i<level.size();i+=2) next.push_back(merkle_parent(level[i],level[i+1]));
        level=std::move(next);
    }
    return level.front();
}

int mod(int x,int p){ x%=p; if(x<0)x+=p; return x; }
int mod_pow(int a,int e,int p){
    int r=1;
    a=mod(a,p);
    while(e){
        if(e&1) r=static_cast<int>((static_cast<long long>(r)*a)%p);
        a=static_cast<int>((static_cast<long long>(a)*a)%p);
        e>>=1;
    }
    return r;
}
int mod_inv(int a,int p){
    require(mod(a,p)!=0,"no inverse for zero");
    return mod_pow(a,p-2,p);
}

struct Share{ int x=0,y=0; };
std::vector<Share> shamir_demo_shares(int secret,int n){
    constexpr int p=257;
    require(secret>=0&&secret<p,"secret outside field");
    require(n>=3&&n<p,"invalid share count");
    const int a1=17,a2=93; // deterministic teaching coefficients; NOT production randomness.
    std::vector<Share> out;
    for(int x=1;x<=n;++x){
        const int y=mod(secret+a1*x+a2*x*x,p);
        out.push_back({x,y});
    }
    return out;
}
int shamir_reconstruct_three(const std::array<Share,3>& s){
    constexpr int p=257;
    long long result=0;
    for(int i=0;i<3;++i){
        int numerator=1,denominator=1;
        for(int j=0;j<3;++j) if(i!=j){
            numerator=mod(numerator*(-s[j].x),p);
            denominator=mod(denominator*(s[i].x-s[j].x),p);
        }
        const int li=mod(numerator*mod_inv(denominator,p),p);
        result+=static_cast<long long>(s[i].y)*li;
    }
    return mod(static_cast<int>(result%p),p);
}

void test_sha256(){
    require(hex(sha256(bytes("abc")))=="ba7816bf8f01cfea414140de5dae2223"
                                         "b00361a396177a9cb410ff61f20015ad",
            "SHA-256 abc known-answer mismatch");
    require(hex(sha256({}))=="e3b0c44298fc1c149afbf4c8996fb924"
                              "27ae41e4649b934ca495991b7852b855",
            "SHA-256 empty known-answer mismatch");
}

void test_hmac(){
    std::vector<std::uint8_t> key(20,0x0b);
    require(hex(hmac_sha256(key,bytes("Hi There")))==
            "b0344c61d8db38535ca8afceaf0bf12b"
            "881dc200c9833da726e9376c2e32cff7",
            "RFC 4231 HMAC-SHA256 test case mismatch");
}

void test_hkdf(){
    const auto ikm=from_hex("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b");
    const auto salt=from_hex("000102030405060708090a0b0c");
    const auto info=from_hex("f0f1f2f3f4f5f6f7f8f9");
    const Digest prk=hkdf_extract(salt,ikm);
    require(hex(prk)=="077709362c2e32df0ddc3f0dc47bba63"
                      "90b6c73bb50f9c3122ec844ad7c2b3e5",
            "RFC 5869 PRK mismatch");
    require(hex(hkdf_expand(prk,info,42))==
            "3cb25f25faacd57a90434f64d0362f2a"
            "2d2d0a90cf1a5a4c5db02d56ecc4c5bf"
            "34007208d5b887185865",
            "RFC 5869 OKM mismatch");
}

void test_merkle(){
    const auto a=merkle_root({bytes("a"),bytes("b"),bytes("c")});
    const auto b=merkle_root({bytes("a"),bytes("b"),bytes("c")});
    const auto c=merkle_root({bytes("a"),bytes("B"),bytes("c")});
    require(a==b,"Merkle root must be deterministic");
    require(a!=c,"changing a leaf must change this Merkle root");
}

void test_shamir(){
    const auto shares=shamir_demo_shares(200,5);
    require(shamir_reconstruct_three({shares[0],shares[1],shares[2]})==200,"Shamir first subset mismatch");
    require(shamir_reconstruct_three({shares[0],shares[2],shares[4]})==200,"Shamir alternate subset mismatch");
}

} // namespace

int main(){
    try{
        test_sha256();
        test_hmac();
        test_hkdf();
        test_merkle();
        test_shamir();
        std::cout<<"crypto_primitives_lab: PASS\n"
                 <<"proved: SHA-256 known-answer vectors, HMAC-SHA256 RFC 4231 vector, HKDF-SHA256 RFC 5869 vector, domain-separated Merkle root sensitivity, 3-of-n finite-field Shamir reconstruction\n"
                 <<"not proved: side-channel resistance, CSPRNG quality for Shamir coefficients, signature schemes, certificate/trust policy, secure key storage, protocol security, or production cryptographic suitability\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"crypto_primitives_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
