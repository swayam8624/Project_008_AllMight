// PROJECTALLMIGHT AES-GCM authenticated-encryption witness using OpenSSL EVP.
// The zero-key/zero-IV vector is a standard GCM known-answer case.
#include <openssl/evp.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

using Ctx=std::unique_ptr<EVP_CIPHER_CTX,decltype(&EVP_CIPHER_CTX_free)>;

struct Sealed{
    std::vector<std::uint8_t> ciphertext;
    std::array<std::uint8_t,16> tag{};
};

Sealed encrypt(
    std::span<const std::uint8_t,16> key,
    std::span<const std::uint8_t> iv,
    std::span<const std::uint8_t> plaintext,
    std::span<const std::uint8_t> aad) {

    require(!iv.empty(),"GCM IV must not be empty");
    Ctx ctx(EVP_CIPHER_CTX_new(),EVP_CIPHER_CTX_free);
    require(ctx!=nullptr,"EVP_CIPHER_CTX_new failed");
    require(EVP_EncryptInit_ex(ctx.get(),EVP_aes_128_gcm(),nullptr,nullptr,nullptr)==1,
            "EVP_EncryptInit_ex cipher failed");
    require(EVP_CIPHER_CTX_ctrl(ctx.get(),EVP_CTRL_AEAD_SET_IVLEN,static_cast<int>(iv.size()),nullptr)==1,
            "set GCM IV length failed");
    require(EVP_EncryptInit_ex(ctx.get(),nullptr,nullptr,key.data(),iv.data())==1,
            "set GCM key/IV failed");

    int n=0;
    if(!aad.empty()){
        require(EVP_EncryptUpdate(ctx.get(),nullptr,&n,aad.data(),static_cast<int>(aad.size()))==1,
                "GCM AAD update failed");
    }

    Sealed out;
    out.ciphertext.resize(plaintext.size()+16);
    int written=0;
    if(!plaintext.empty()){
        require(EVP_EncryptUpdate(ctx.get(),out.ciphertext.data(),&n,plaintext.data(),static_cast<int>(plaintext.size()))==1,
                "GCM plaintext update failed");
        written=n;
    }
    require(EVP_EncryptFinal_ex(ctx.get(),out.ciphertext.data()+written,&n)==1,
            "GCM encrypt final failed");
    written+=n;
    out.ciphertext.resize(static_cast<std::size_t>(written));

    require(EVP_CIPHER_CTX_ctrl(ctx.get(),EVP_CTRL_AEAD_GET_TAG,
                                static_cast<int>(out.tag.size()),out.tag.data())==1,
            "get GCM tag failed");
    return out;
}

bool decrypt(
    std::span<const std::uint8_t,16> key,
    std::span<const std::uint8_t> iv,
    std::span<const std::uint8_t> ciphertext,
    std::span<const std::uint8_t> aad,
    std::span<const std::uint8_t,16> tag,
    std::vector<std::uint8_t>& plaintext) {

    Ctx ctx(EVP_CIPHER_CTX_new(),EVP_CIPHER_CTX_free);
    require(ctx!=nullptr,"EVP_CIPHER_CTX_new failed");
    require(EVP_DecryptInit_ex(ctx.get(),EVP_aes_128_gcm(),nullptr,nullptr,nullptr)==1,
            "EVP_DecryptInit_ex cipher failed");
    require(EVP_CIPHER_CTX_ctrl(ctx.get(),EVP_CTRL_AEAD_SET_IVLEN,static_cast<int>(iv.size()),nullptr)==1,
            "set GCM IV length failed");
    require(EVP_DecryptInit_ex(ctx.get(),nullptr,nullptr,key.data(),iv.data())==1,
            "set GCM key/IV failed");

    int n=0;
    if(!aad.empty()){
        require(EVP_DecryptUpdate(ctx.get(),nullptr,&n,aad.data(),static_cast<int>(aad.size()))==1,
                "GCM AAD update failed");
    }

    plaintext.assign(ciphertext.size()+16,0);
    int written=0;
    if(!ciphertext.empty()){
        require(EVP_DecryptUpdate(ctx.get(),plaintext.data(),&n,ciphertext.data(),static_cast<int>(ciphertext.size()))==1,
                "GCM ciphertext update failed");
        written=n;
    }

    std::array<std::uint8_t,16> mutable_tag{};
    std::memcpy(mutable_tag.data(),tag.data(),tag.size());
    require(EVP_CIPHER_CTX_ctrl(ctx.get(),EVP_CTRL_AEAD_SET_TAG,
                                static_cast<int>(mutable_tag.size()),mutable_tag.data())==1,
            "set expected GCM tag failed");

    const int ok=EVP_DecryptFinal_ex(ctx.get(),plaintext.data()+written,&n);
    if(ok!=1){
        plaintext.clear(); // unauthenticated plaintext must not escape the API.
        return false;
    }
    written+=n;
    plaintext.resize(static_cast<std::size_t>(written));
    return true;
}

std::vector<std::uint8_t> hex(const std::string& s){
    require(s.size()%2==0,"hex string must have even length");
    std::vector<std::uint8_t> out(s.size()/2);
    auto nibble=[](char c)->int{
        if(c>='0'&&c<='9') return c-'0';
        if(c>='a'&&c<='f') return 10+c-'a';
        if(c>='A'&&c<='F') return 10+c-'A';
        return -1;
    };
    for(std::size_t i=0;i<out.size();++i){
        const int hi=nibble(s[2*i]),lo=nibble(s[2*i+1]);
        require(hi>=0&&lo>=0,"invalid hex");
        out[i]=static_cast<std::uint8_t>((hi<<4)|lo);
    }
    return out;
}

void test_known_answer(){
    const std::array<std::uint8_t,16> key{};
    const std::array<std::uint8_t,12> iv{};
    const std::array<std::uint8_t,16> plaintext{};

    const Sealed s=encrypt(key,iv,plaintext,{});
    const auto expected_ct=hex("0388dace60b6a392f328c2b971b2fe78");
    const auto expected_tag=hex("ab6e47d42cec13bdf53a67b21257bddf");
    require(s.ciphertext==expected_ct,"AES-128-GCM known-answer ciphertext mismatch");
    require(std::equal(s.tag.begin(),s.tag.end(),expected_tag.begin(),expected_tag.end()),
            "AES-128-GCM known-answer tag mismatch");

    std::vector<std::uint8_t> opened;
    require(decrypt(key,iv,s.ciphertext,{},s.tag,opened),"known-answer decrypt authentication failed");
    require(opened==std::vector<std::uint8_t>(plaintext.begin(),plaintext.end()),
            "known-answer plaintext mismatch");

    auto bad_tag=s.tag;
    bad_tag[0]^=0x80;
    require(!decrypt(key,iv,s.ciphertext,{},bad_tag,opened),
            "tampered tag must fail authentication");
    require(opened.empty(),"unauthenticated plaintext must be discarded");
}

void test_aad_and_nonce_contract(){
    const std::array<std::uint8_t,16> key{
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f};
    const std::array<std::uint8_t,12> iv1{0,1,2,3,4,5,6,7,8,9,10,11};
    const std::array<std::uint8_t,12> iv2{0,1,2,3,4,5,6,7,8,9,10,12};
    const std::vector<std::uint8_t> plain{'a','s','s','e','t','-','p','a','y','l','o','a','d'};
    const std::vector<std::uint8_t> aad{'m','a','n','i','f','e','s','t',':','v','1'};

    const Sealed a=encrypt(key,iv1,plain,aad);
    const Sealed b=encrypt(key,iv2,plain,aad);
    require(a.ciphertext!=b.ciphertext || a.tag!=b.tag,
            "changing nonce must change authenticated output");

    std::vector<std::uint8_t> opened;
    require(decrypt(key,iv1,a.ciphertext,aad,a.tag,opened)&&opened==plain,
            "AAD round trip failed");

    auto wrong_aad=aad;
    wrong_aad.back()='2';
    require(!decrypt(key,iv1,a.ciphertext,wrong_aad,a.tag,opened),
            "AAD tamper must fail authentication");
    require(opened.empty(),"plaintext from failed AAD authentication must be discarded");
}

} // namespace

int main(){
    try{
        test_known_answer();
        test_aad_and_nonce_contract();
        std::cout
            <<"openssl_aes_gcm_smoke: PASS\n"
            <<"proved: AES-128-GCM known-answer ciphertext/tag, authenticated decrypt, tag tamper rejection, AAD binding, nonce-sensitive output, discard-on-auth-failure API behavior\n"
            <<"not proved: nonce generation/storage policy, key lifecycle/HSMs, side-channel/FIPS configuration, protocol replay defense, large-message streaming, or ReliQuary production parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"openssl_aes_gcm_smoke: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
