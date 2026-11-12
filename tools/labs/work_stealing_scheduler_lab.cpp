// PROJECTALLMIGHT work-stealing scheduler reference.
// Uses mutex-protected local deques to prove ownership/steal semantics without claiming lock-free Chase-Lev behavior.
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

struct LocalDeque{
    std::mutex mutex;
    std::deque<std::function<void()>> jobs;

    void push_local(std::function<void()> f){
        std::scoped_lock lock(mutex);
        jobs.push_back(std::move(f));
    }
    bool pop_local(std::function<void()>& out){
        std::scoped_lock lock(mutex);
        if(jobs.empty()) return false;
        out=std::move(jobs.back());
        jobs.pop_back();
        return true;
    }
    bool steal(std::function<void()>& out){
        std::scoped_lock lock(mutex);
        if(jobs.empty()) return false;
        out=std::move(jobs.front());
        jobs.pop_front();
        return true;
    }
};

void test_end_semantics(){
    LocalDeque q;
    std::vector<int> order;
    q.push_local([&]{order.push_back(1);});
    q.push_local([&]{order.push_back(2);});
    q.push_local([&]{order.push_back(3);});
    std::function<void()> f;
    require(q.pop_local(f),"owner pop failed"); f();
    require(q.steal(f),"steal failed"); f();
    require(q.pop_local(f),"second owner pop failed"); f();
    require(order==std::vector<int>({3,1,2}),"owner must pop LIFO while thief steals FIFO");
}

void test_parallel_stealing(){
    constexpr int workers=4;
    constexpr int tasks=600;
    std::vector<LocalDeque> queues(workers);
    std::atomic<int> executed{0};
    std::atomic<int> stolen{0};
    std::atomic<long long> sum{0};

    // Deliberately load queue 0. Workers 1..3 can only make progress by stealing.
    long long expected=0;
    for(int i=1;i<=tasks;++i){
        expected+=i;
        queues[0].push_local([&,i]{
            sum.fetch_add(i,std::memory_order_relaxed);
            executed.fetch_add(1,std::memory_order_relaxed);
        });
    }

    std::vector<std::jthread> threads;
    for(int id=0;id<workers;++id){
        threads.emplace_back([&,id]{
            while(executed.load(std::memory_order_relaxed)<tasks){
                std::function<void()> job;
                bool got=queues[id].pop_local(job);
                if(!got){
                    for(int step=1;step<workers&&!got;++step){
                        const int victim=(id+step)%workers;
                        if(queues[victim].steal(job)){
                            stolen.fetch_add(1,std::memory_order_relaxed);
                            got=true;
                        }
                    }
                }
                if(got) job();
                else std::this_thread::yield();
            }
        });
    }
    for(auto& t:threads) t.join();

    require(executed.load()==tasks,"task executed zero or multiple times");
    require(sum.load()==expected,"work-stealing scheduler lost or duplicated task payload");
    require(stolen.load()>0,"imbalanced test produced no steals");
}

} // namespace

int main(){
    try{
        test_end_semantics();
        test_parallel_stealing();
        std::cout<<"work_stealing_scheduler_lab: PASS\n"
                 <<"proved: owner-LIFO/thief-FIFO deque contract, cross-worker stealing under forced imbalance, exactly-once task count and deterministic integer payload total\n"
                 <<"not proved: lock-free Chase-Lev memory ordering, affinity/NUMA policy, priorities, dependency DAG scheduling, cancellation, exception futures, or production job-system performance\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"work_stealing_scheduler_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
