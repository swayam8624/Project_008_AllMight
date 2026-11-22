// PROJECTALLMIGHT GPU-parallel-algorithm CPU oracle.
// Simulates block scan, predicate-scan-scatter compaction, histograms and stable LSD radix sorting.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

std::vector<std::uint32_t> exclusive_scan(const std::vector<std::uint32_t>& in){
    std::vector<std::uint32_t> out(in.size());
    std::uint64_t sum=0;
    for(std::size_t i=0;i<in.size();++i){
        require(sum<=UINT32_MAX,"scan offset overflow");
        out[i]=static_cast<std::uint32_t>(sum);
        sum+=in[i];
    }
    require(sum<=UINT32_MAX,"scan total overflow");
    return out;
}

// CPU model of a hierarchical GPU scan: local block scans, scan block totals, add block bases.
std::vector<std::uint32_t> hierarchical_scan(
    const std::vector<std::uint32_t>& in,
    std::size_t block_size) {
    require(block_size>0,"block size must be positive");
    if(in.empty()) return {};

    const std::size_t blocks=(in.size()+block_size-1)/block_size;
    std::vector<std::uint32_t> out(in.size(),0);
    std::vector<std::uint32_t> totals(blocks,0);

    for(std::size_t b=0;b<blocks;++b){
        const std::size_t begin=b*block_size;
        const std::size_t end=std::min(in.size(),begin+block_size);
        std::uint64_t local=0;
        for(std::size_t i=begin;i<end;++i){
            require(local<=UINT32_MAX,"local scan overflow");
            out[i]=static_cast<std::uint32_t>(local);
            local+=in[i];
        }
        require(local<=UINT32_MAX,"block total overflow");
        totals[b]=static_cast<std::uint32_t>(local);
    }

    const auto bases=exclusive_scan(totals);
    for(std::size_t b=0;b<blocks;++b){
        const std::size_t begin=b*block_size;
        const std::size_t end=std::min(in.size(),begin+block_size);
        for(std::size_t i=begin;i<end;++i){
            const std::uint64_t value=static_cast<std::uint64_t>(out[i])+bases[b];
            require(value<=UINT32_MAX,"global scan overflow");
            out[i]=static_cast<std::uint32_t>(value);
        }
    }
    return out;
}

template<class T,class Pred>
std::vector<T> compact(const std::vector<T>& values,Pred pred){
    std::vector<std::uint32_t> flags(values.size());
    for(std::size_t i=0;i<values.size();++i) flags[i]=pred(values[i])?1u:0u;
    const auto offsets=hierarchical_scan(flags,7);
    const std::uint32_t count=values.empty()?0u:offsets.back()+flags.back();
    std::vector<T> out(count);
    for(std::size_t i=0;i<values.size();++i) if(flags[i]) out[offsets[i]]=values[i];
    return out;
}

std::array<std::uint32_t,256> byte_histogram(const std::vector<std::uint32_t>& keys,int shift){
    require(shift==0||shift==8||shift==16||shift==24,"radix shift must select one byte");
    std::array<std::uint32_t,256> h{};
    for(std::uint32_t k:keys) ++h[(k>>shift)&0xffu];
    return h;
}

std::vector<std::uint32_t> radix_sort_u32(std::vector<std::uint32_t> keys){
    std::vector<std::uint32_t> tmp(keys.size());
    for(int shift:{0,8,16,24}){
        const auto h=byte_histogram(keys,shift);
        std::vector<std::uint32_t> counts(h.begin(),h.end());
        const auto offsets=exclusive_scan(counts);
        auto cursor=offsets;
        for(std::uint32_t k:keys){
            const std::uint32_t bucket=(k>>shift)&0xffu;
            tmp[cursor[bucket]++]=k;
        }
        keys.swap(tmp);
    }
    return keys;
}

void test_scan(){
    std::mt19937 rng(20261112);
    std::uniform_int_distribution<int> n_dist(0,200);
    std::uniform_int_distribution<int> v_dist(0,9);
    for(int trial=0;trial<500;++trial){
        const int n=n_dist(rng);
        std::vector<std::uint32_t> v(static_cast<std::size_t>(n));
        for(auto& x:v) x=static_cast<std::uint32_t>(v_dist(rng));
        const auto ref=exclusive_scan(v);
        for(std::size_t block:{1u,2u,3u,7u,32u,64u,257u})
            require(hierarchical_scan(v,block)==ref,"hierarchical scan differs from serial oracle");
    }
}

void test_compaction(){
    std::vector<int> v(100);
    std::iota(v.begin(),v.end(),0);
    const auto out=compact(v,[](int x){return x%3==1;});
    std::vector<int> ref;
    std::copy_if(v.begin(),v.end(),std::back_inserter(ref),[](int x){return x%3==1;});
    require(out==ref,"scan/scatter compaction lost stable order");
}

void test_radix(){
    std::mt19937 rng(7);
    std::vector<std::uint32_t> keys(4000);
    for(auto& k:keys) k=rng();
    // Add duplicates and extreme keys.
    keys.push_back(0); keys.push_back(UINT32_MAX); keys.push_back(17); keys.push_back(17);
    auto ref=keys;
    std::stable_sort(ref.begin(),ref.end());
    require(radix_sort_u32(keys)==ref,"LSD radix sort disagrees with stable numeric order");

    const auto h=byte_histogram(ref,0);
    const std::uint64_t total=std::accumulate(h.begin(),h.end(),std::uint64_t{0});
    require(total==ref.size(),"histogram count does not equal input count");
}

} // namespace

int main(){
    try{
        test_scan(); test_compaction(); test_radix();
        std::cout<<"gpu_parallel_primitives_lab: PASS\n"
                 <<"proved: hierarchical exclusive scan parity over varied block sizes, stable predicate-scan-scatter compaction, byte histograms, stable four-pass u32 LSD radix sort\n"
                 <<"not proved: GPU barriers/subgroup implementation, bank conflicts, occupancy, device-wide scan scheduling, bitonic sort, hash tables, indirect command encoding, or hardware performance\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"gpu_parallel_primitives_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
