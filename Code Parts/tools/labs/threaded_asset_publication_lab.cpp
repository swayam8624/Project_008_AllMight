// C++20 reference: real worker threads plus generation-safe asset publication.
// GPU upload and render fences are deterministic models, not Vulkan/Metal calls.
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace asset_lab {
void demand(bool ok,std::string_view why) {
    if (!ok) throw std::runtime_error(std::string(why));
}
// A deliberately tiny trusted-size package format. The byte count in its
// header is NOT trusted until checked against the actual encoded buffer.
std::vector<std::uint8_t> pack(const std::vector<std::uint8_t>& bytes) {
    demand(bytes.size()<=32,"fixture payload too large");
    std::vector<std::uint8_t> out;
    const auto n=static_cast<std::uint32_t>(bytes.size());
    for (int i=0;i<4;i++) out.push_back(static_cast<std::uint8_t>(n>>(8*i)));
    std::uint8_t check=0;
    for (auto b:bytes) {out.push_back(b);check^=b;}
    out.push_back(check);
    return out;
}
struct Decode {
    bool valid=false;
    std::vector<std::uint8_t> bytes;
    std::string error;
};
Decode unpack(const std::vector<std::uint8_t>& raw) {
    if (raw.size()<5) return {false,{},"truncated package header"};
    std::uint32_t n=0;
    for (unsigned i=0;i<4;i++) n|=std::uint32_t(raw[i])<<(8*i);
    // Check BOTH a semantic maximum and the encoded extent before allocation.
    if (n>32 || std::size_t(n)!=raw.size()-5)
        return {false,{},"payload size mismatch"};
    std::uint8_t check=0;
    for (std::size_t i=4;i<raw.size()-1;i++) check^=raw[i];
    if (check!=raw.back()) return {false,{},"checksum mismatch"};
    return {true,std::vector<std::uint8_t>(raw.begin()+4,raw.end()-1),{}};
}

struct Job {
    std::string asset;
    std::uint64_t generation=0;
    int priority=0;
    std::vector<std::uint8_t> encoded; // Owned job input; no borrowed caller span.
    std::shared_future<void> may_finish; // Optional controlled test gate.
    std::shared_ptr<std::promise<void>> started;
};
struct Completion {
    std::string asset;
    std::uint64_t generation=0;
    Decode result;
};

class DecoderPool {
    std::mutex guard_;
    std::condition_variable work_ready_,completion_ready_;
    std::deque<Job> queued_;
    std::deque<Completion> completed_;
    bool closing_=false;
    std::vector<std::thread> workers_;
    void work() {
        for (;;) {
            Job job;
            {
                std::unique_lock lock(guard_);
                work_ready_.wait(lock,[&]{return closing_ || !queued_.empty();});
                if (closing_ && queued_.empty()) return;
                // Highest priority first among jobs not yet started. In-flight
                // work cannot be preempted by changing a later request priority.
                auto best=std::max_element(queued_.begin(),queued_.end(),
                    [](const Job& a,const Job& b){return a.priority<b.priority;});
                job=std::move(*best);
                queued_.erase(best);
            }
            if (job.started) job.started->set_value();
            if (job.may_finish.valid()) job.may_finish.wait();
            Completion done{std::move(job.asset),job.generation,unpack(job.encoded)};
            {
                std::lock_guard lock(guard_);
                completed_.push_back(std::move(done));
            }
            completion_ready_.notify_one();
        }
    }
public:
    explicit DecoderPool(unsigned concurrency) {
        demand(concurrency>0 && concurrency<=8,"invalid worker count");
        for (unsigned i=0;i<concurrency;i++)
            workers_.emplace_back([this]{work();});
    }
    DecoderPool(const DecoderPool&)=delete;
    DecoderPool& operator=(const DecoderPool&)=delete;
    ~DecoderPool() {
        // Caller must release any external test gates before destruction.
        {std::lock_guard lock(guard_);closing_=true;}
        work_ready_.notify_all();
        for (auto& worker:workers_)worker.join();
    }
    void submit(Job job) {
        {std::lock_guard lock(guard_);
            demand(!closing_,"queue closed");queued_.push_back(std::move(job));}
        work_ready_.notify_one();
    }
    Completion await_completion() {
        std::unique_lock lock(guard_);
        demand(completion_ready_.wait_for(lock,std::chrono::seconds(5),
               [&]{return !completed_.empty();}),"worker completion timed out");
        Completion out=std::move(completed_.front());
        completed_.pop_front();
        return out;
    }
};

struct Resident {
    std::vector<std::uint8_t> bytes;
    std::uint64_t last_draw_fence=0;
};
struct Retired {
    std::size_t bytes=0;
    std::uint64_t last_draw_fence=0;
};
struct Slot {
    std::uint64_t generation=0;
    std::size_t pending_estimate=0;
    bool pending=false;
    bool cpu_ready=false;
    std::vector<std::uint8_t> decoded;
    std::optional<Resident> current;
};
class Residency {
    std::map<std::string,Slot> slots_;
    std::vector<Retired> retired_;
    const std::size_t budget_;
    std::size_t used_=0;
    std::uint64_t completed_fence_=0;
    unsigned stale_completions_=0;
    unsigned failed_packages_=0;
    void retire_or_free(Resident&& old) {
        if (old.last_draw_fence<=completed_fence_) used_-=old.bytes.size();
        else retired_.push_back({old.bytes.size(),old.last_draw_fence});
    }
public:
    explicit Residency(std::size_t limit):budget_(limit) {}
    // request() is called by the owning gameplay/resource thread only.
    std::uint64_t request(const std::string& name,std::size_t estimated_bytes) {
        Slot& s=slots_[name];
        if (s.pending) cancel(name);
        demand(estimated_bytes<=budget_-used_,"budget rejected request");
        used_+=estimated_bytes;
        s.pending=true;s.cpu_ready=false;s.pending_estimate=estimated_bytes;
        s.decoded.clear();
        return ++s.generation;
    }
    void cancel(const std::string& name) {
        auto it=slots_.find(name);
        demand(it!=slots_.end() && it->second.pending,"cancel nonpending");
        Slot& s=it->second;
        used_-=s.pending_estimate;
        s.pending=false;s.cpu_ready=false;s.pending_estimate=0;
        s.decoded.clear();
        ++s.generation; // Any worker may still finish, but may not publish.
    }
    bool accept(Completion done) {
        auto it=slots_.find(done.asset);
        if (it==slots_.end() || !it->second.pending ||
            it->second.generation!=done.generation) {
            ++stale_completions_;return false;
        }
        Slot& s=it->second;
        if (!done.result.valid || done.result.bytes.size()>s.pending_estimate) {
            ++failed_packages_;cancel(done.asset);return false;
        }
        s.decoded=std::move(done.result.bytes);
        s.cpu_ready=true;
        return true;
    }
    // Called only after a real renderer would prove its new upload complete.
    bool publish(const std::string& name) {
        auto it=slots_.find(name);
        if (it==slots_.end()) return false;
        Slot& s=it->second;
        if (!s.pending || !s.cpu_ready) return false;
        const std::size_t actual=s.decoded.size();
        // The estimate was precharged. Transfer its charge to actual residency.
        used_-=s.pending_estimate-actual;
        s.pending=false;s.cpu_ready=false;s.pending_estimate=0;
        if (s.current) retire_or_free(std::move(*s.current));
        s.current=Resident{std::move(s.decoded),0};
        return true;
    }
    std::optional<std::vector<std::uint8_t>> read(const std::string& name) const {
        auto it=slots_.find(name);
        if (it==slots_.end() || !it->second.current) return std::nullopt;
        return it->second.current->bytes;
    }
    void drawn(const std::string& name,std::uint64_t fence) {
        auto it=slots_.find(name);
        demand(it!=slots_.end() && it->second.current,"draw unresolved asset");
        demand(fence>completed_fence_,"cannot submit to an already completed fence");
        it->second.current->last_draw_fence=
            std::max(fence,it->second.current->last_draw_fence);
    }
    void gpu_completed(std::uint64_t fence) {
        demand(fence>=completed_fence_,"GPU completion clock reversed");
        completed_fence_=fence;
        auto it=retired_.begin();
        while (it!=retired_.end()) {
            if (it->last_draw_fence<=fence) {
                used_-=it->bytes;
                it=retired_.erase(it);
            } else ++it;
        }
    }
    void evict(const std::string& name) {
        Slot& s=slots_.at(name);
        demand(!s.pending && s.current.has_value(),"cannot evict pending/missing");
        retire_or_free(std::move(*s.current));
        s.current.reset();
        ++s.generation;
    }
    std::size_t charged()const{return used_;}
    std::size_t retired_count()const{return retired_.size();}
    unsigned stale()const{return stale_completions_;}
    unsigned failed()const{return failed_packages_;}
};
} // namespace asset_lab

int main() {
    using namespace asset_lab;
    DecoderPool workers(2);
    Residency budget(32);
    const std::string name="cell7/door/albedo";
    std::promise<void> release_old;
    const auto old_gate=release_old.get_future().share();
    auto started=std::make_shared<std::promise<void>>();
    const auto begun=started->get_future();
    const auto gen1=budget.request(name,12);
    workers.submit({name,gen1,1,pack({1,2,3,4}),old_gate,started});
    demand(begun.wait_for(std::chrono::seconds(5))==std::future_status::ready,
           "old request did not start");
    budget.cancel(name); // Worker 1 is already decoding generation 1.
    const auto gen3=budget.request(name,8); // A newer request reuses the slot.
    workers.submit({name,gen3,10,pack({9,8,7,6}),{},nullptr});
    const auto next=workers.await_completion(); // Worker 2 can finish first.
    demand(next.generation==gen3 && budget.accept(next),"new decode not accepted");
    demand(budget.publish(name),"new asset publication failed");
    demand(budget.read(name).value()==std::vector<std::uint8_t>({9,8,7,6}),
           "wrong bytes published");
    release_old.set_value();
    const auto late=workers.await_completion();
    demand(late.generation==gen1 && !budget.accept(late) && budget.stale()==1,
           "old completion overwrote newer asset");
    demand(budget.charged()==4,"cancelled request leaked reserved bytes");

    budget.drawn(name,5); // Old generation is referenced by GPU work.
    const auto gen4=budget.request(name,8);
    workers.submit({name,gen4,5,pack({4,3,2}),{},nullptr});
    demand(budget.accept(workers.await_completion()),"replacement decode failed");
    demand(budget.publish(name),"replacement publish failed");
    demand(budget.retired_count()==1 && budget.charged()==7,
           "old GPU resource freed before completion");
    budget.gpu_completed(4);
    demand(budget.charged()==7,"premature GPU resource retirement");
    budget.gpu_completed(5);
    demand(budget.retired_count()==0 && budget.charged()==3,
           "completed GPU frame did not retire its resource");

    // Malformed bytes must not replace an existing valid published generation.
    const auto malformed=budget.request(name,8);
    auto invalid=pack({1,1,1});invalid.back()^=0xff;
    workers.submit({name,malformed,1,invalid,{},nullptr});
    demand(!budget.accept(workers.await_completion()) && budget.failed()==1,
           "bad checksum accepted");
    demand(budget.read(name).value()==std::vector<std::uint8_t>({4,3,2}),
           "malformed hot reload replaced good bytes");
    bool rejected=false;
    try { (void)budget.request("huge",32); }
    catch (const std::runtime_error&) {rejected=true;}
    demand(rejected && budget.charged()==3,
           "rejected over-budget request changed accounting");
    budget.evict(name);
    demand(budget.charged()==0,"asset not released after last user");
    std::cout << "PASS actual workers, stale completion, owned bytes, "
                 "checksum, hot reload, budget and mock GPU-fence retirement\n";
}
