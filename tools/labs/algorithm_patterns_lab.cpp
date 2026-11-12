// PROJECTALLMIGHT Phase 5 witness: two-pointer and sliding-window patterns with explicit domain contracts.
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

std::optional<std::pair<std::size_t,std::size_t>>
two_sum_sorted(const std::vector<int>& a,int target){
    if(a.size()<2) return std::nullopt;
    require(std::is_sorted(a.begin(),a.end()),"two-pointer pair search requires sorted input");
    std::size_t left=0,right=a.size()-1;
    while(left<right){
        const long long sum=static_cast<long long>(a[left])+a[right];
        if(sum==target) return std::pair{left,right};
        if(sum<target) ++left;
        else --right;
    }
    return std::nullopt;
}

// Longest contiguous window whose sum is <= budget.
// Monotonic shrink logic is valid because all input values are nonnegative.
std::pair<std::size_t,std::size_t>
longest_nonnegative_window_at_most(const std::vector<int>& a,long long budget){
    require(budget>=0,"budget must be nonnegative");
    for(int x:a) require(x>=0,"sliding-window monotonicity requires nonnegative values");

    std::size_t left=0,best_left=0,best_len=0;
    long long sum=0;
    for(std::size_t right=0;right<a.size();++right){
        sum+=a[right];
        while(sum>budget && left<=right){
            sum-=a[left++];
        }
        const std::size_t len=right-left+1;
        if(len>best_len){
            best_len=len;
            best_left=left;
        }
    }
    return {best_left,best_len};
}

std::pair<std::size_t,std::size_t>
brute_window(const std::vector<int>& a,long long budget){
    std::size_t bl=0,bn=0;
    for(std::size_t i=0;i<a.size();++i){
        long long sum=0;
        for(std::size_t j=i;j<a.size();++j){
            sum+=a[j];
            if(sum<=budget && j-i+1>bn){ bl=i;bn=j-i+1; }
        }
    }
    return {bl,bn};
}

void test_two_pointer(){
    const std::vector<int> a{-5,-1,2,4,7,10};
    auto p=two_sum_sorted(a,9);
    require(p.has_value(),"expected pair");
    require(a[p->first]+a[p->second]==9,"two-pointer pair sum wrong");
    require(!two_sum_sorted(a,99).has_value(),"impossible pair should be absent");
}

void test_window_against_brute(){
    const std::vector<std::vector<int>> cases{
        {1,2,3,4,5},
        {0,0,0,5,0,0},
        {4,1,1,1,7,1,2},
        {},
        {8}
    };
    for(const auto& a:cases){
        for(long long budget=0;budget<=12;++budget){
            require(longest_nonnegative_window_at_most(a,budget)==brute_window(a,budget),
                    "sliding-window result differs from brute-force oracle");
        }
    }

    bool rejected=false;
    try{ (void)longest_nonnegative_window_at_most({2,-1,2},3); }
    catch(const std::runtime_error&){ rejected=true; }
    require(rejected,"negative input must reject this monotone-window algorithm");
}

} // namespace

int main(){
    try{
        test_two_pointer();
        test_window_against_brute();
        std::cout<<"algorithm_patterns_lab: PASS\n"
                 <<"proved: sorted two-pointer pair search and nonnegative sliding window differential-tested against brute force\n"
                 <<"not proved: arbitrary signed-window problems, deque-based monotone windows, string-window frequency maps, or production workload superiority\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"algorithm_patterns_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
