// Phase 4 (M019-M024, original Parts 301-400) -- CPU systems witness.
// This is one reproducible CPU exercise, NOT an emulation of CPU caches,
// actual SIMD instruction selection, OS scheduling, or GPU resource fences.
// Assertions here deliberately remain active with -DNDEBUG.
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <limits>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <numeric>
#include <span>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace allmight::phase4 {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

// A "cold" payload is deliberately carried in each AoS record but never read
// by the hot update. This increases bytes transferred per sequential scan.
struct ParticleAoS {
    float x{}, y{}, z{}, mass{};
    std::array<float, 12> debug_and_editor_metadata{};
};

struct ParticleSoA {
    std::vector<float> x, y, z, mass;
    explicit ParticleSoA(std::size_t count)
        : x(count), y(count), z(count), mass(count) {}
};

// Values chosen to be exactly representable in float. Identical output is a
// correctness witness, not proof that one layout is always faster.
void test_layout_and_dense_iteration() {
    constexpr std::size_t count = 4096;
    std::vector<ParticleAoS> aos(count);
    ParticleSoA soa(count);
    for (std::size_t i = 0; i < count; ++i) {
        const float f = static_cast<float>(i % 16);
        aos[i].x = soa.x[i] = f;
        aos[i].y = soa.y[i] = 2.0F;
        aos[i].z = soa.z[i] = 3.0F;
        aos[i].mass = soa.mass[i] = 1.0F;
    }

    float sum_aos = 0.0F;
    float sum_soa = 0.0F;
    for (const ParticleAoS& p : aos) sum_aos += p.x * p.mass;
    for (std::size_t i = 0; i < count; ++i)
        sum_soa += soa.x[i] * soa.mass[i];
    require(sum_aos == sum_soa && sum_soa == 30720.0F,
            "AoS and SoA must agree on exactly representable input");

    // Dense compaction: select the hot subset into a contiguous vector.
    std::vector<std::uint32_t> active;
    for (std::uint32_t i = 0; i < count; ++i)
        if ((i % 4) == 0) active.push_back(i);
    require(active.size() == count / 4, "compact active count");
    require(active.front() == 0 && active.back() == count - 4,
            "compacted positions preserve ascending input order");
}

// A fixed 64-byte alignment is a hypothesis about a machine's likely
// destructive interference granularity, not an ISO C++ cache-line promise.
// Use profiler/hardware measurements before claiming a speed improvement.
struct alignas(64) PaddedCounter {
    std::atomic<std::uint64_t> value{0};
};
static_assert(alignof(PaddedCounter) >= 64);

void test_padding_and_atomic_increment() {
    PaddedCounter left, right;
    auto increment = [](PaddedCounter& counter) {
        for (std::uint64_t i = 0; i < 10000; ++i)
            counter.value.fetch_add(1, std::memory_order_relaxed);
    };
    std::thread a(increment, std::ref(left));
    std::thread b(increment, std::ref(right));
    a.join(); b.join();
    require(left.value.load(std::memory_order_relaxed) == 10000,
            "first atomic counter");
    require(right.value.load(std::memory_order_relaxed) == 10000,
            "second atomic counter");
}

// In an acquiring thread, observing ready=true after the release store
// synchronizes the earlier non-atomic payload write. Atomicity of ready by
// itself is not a substitute for correct publication and lifetime.
void test_release_acquire_message() {
    struct Message {
        int payload{};
        std::atomic<bool> ready{false};
    } message;
    std::thread producer([&] {
        message.payload = 42;
        message.ready.store(true, std::memory_order_release);
    });
    std::thread consumer([&] {
        while (!message.ready.load(std::memory_order_acquire))
            std::this_thread::yield();
        require(message.payload == 42, "published payload");
    });
    producer.join(); consumer.join();
}

// A counting upstream allows real allocation observations without claiming
// anything about fragment size, allocator internals, OS pages or NUMA.
class CountingResource final : public std::pmr::memory_resource {
    std::pmr::memory_resource* upstream_ = std::pmr::new_delete_resource();
public:
    std::size_t allocations = 0, deallocations = 0, bytes = 0;
private:
    void* do_allocate(std::size_t n, std::size_t a) override {
        void* p = upstream_->allocate(n, a); // May throw; count on success.
        ++allocations;
        bytes += n;
        return p;
    }
    void do_deallocate(void* p, std::size_t n, std::size_t a) override {
        upstream_->deallocate(p, n, a);
        ++deallocations;
    }
    bool do_is_equal(const std::pmr::memory_resource& other) const
        noexcept override {
        return this == &other;
    }
};

void test_allocators_and_frame_arena() {
    CountingResource resource;
    {
        std::pmr::vector<std::uint64_t> storage{&resource};
        storage.reserve(128);
        for (std::uint64_t i = 0; i < 128; ++i)
            storage.push_back(i);
        require(resource.allocations >= 1 && resource.bytes >= 1024,
                "allocator must provide vector storage");
    }
    require(resource.deallocations == resource.allocations,
            "nonmonotonic PMR storage released after container death");

    // A stack-owned byte buffer feeds an arena. Upstream null resource means
    // exhausting this local space throws instead of allocating elsewhere.
    alignas(std::max_align_t) std::array<std::byte, 2048> memory{};
    std::pmr::monotonic_buffer_resource frame{
        memory.data(), memory.size(), std::pmr::null_memory_resource()};
    std::pmr::vector<int> frame_values{&frame};
    frame_values.resize(100, 7);
    require(frame_values.front() == 7 && frame_values.back() == 7,
            "frame-arena values");
    bool exhausted = false;
    try {
        frame_values.reserve(1000000);
    } catch (const std::bad_alloc&) {
        exhausted = true;
    }
    require(exhausted, "bounded monotonic arena rejects overflow");
    // Do NOT let frame_values escape past the arena's lifetime.
}

// This models CPU timeline tickets only. A real GPU implementation must use
// actual queue completion primitives and respect device loss/resets.
struct CpuRetirement {
    struct Entry {
        std::uint64_t ticket;
        std::unique_ptr<int> resource;
    };
    std::deque<Entry> pending;
    std::uint64_t last_ticket = 0, last_completed = 0;
    std::size_t released = 0;

    void retire(std::uint64_t ticket, std::unique_ptr<int> ptr) {
        if (!ptr || ticket == 0 || ticket < last_ticket)
            throw std::invalid_argument("invalid monotonic retirement");
        pending.push_back(Entry{ticket, std::move(ptr)});
        last_ticket = ticket;
    }
    void complete(std::uint64_t completed) {
        if (completed < last_completed || completed > last_ticket)
            throw std::invalid_argument("invalid completed ticket");
        last_completed = completed;
        while (!pending.empty() && pending.front().ticket <= completed) {
            pending.pop_front(); // Unique pointer releases the owned value.
            ++released;
        }
    }
};

void test_deferred_retirement() {
    CpuRetirement q;
    q.retire(2, std::make_unique<int>(11));
    q.retire(5, std::make_unique<int>(22));
    q.retire(5, std::make_unique<int>(33)); // Same completion ticket is valid.
    q.complete(2);
    require(q.released == 1 && q.pending.size() == 2,
            "must not retire future resources");
    q.complete(5);
    require(q.released == 3 && q.pending.empty(), "release completed resources");
    bool invalid = false;
    try { q.complete(4); }
    catch (const std::invalid_argument&) { invalid = true; }
    require(invalid, "completion timeline cannot reverse");
}

// This is a bounded shared-queue pool, NOT a work-stealing deque. A task
// throws into its packaged_task future; workers do not let it escape.
class JobPool {
    std::mutex mutex_;
    std::condition_variable changed_;
    std::deque<std::function<void()>> tasks_;
    std::vector<std::jthread> workers_;
    bool stopping_ = false;

    void worker() {
        for (;;) {
            std::function<void()> job;
            {
                std::unique_lock lock(mutex_);
                changed_.wait(lock, [&] {
                    return stopping_ || !tasks_.empty();
                });
                if (stopping_ && tasks_.empty()) return;
                job = std::move(tasks_.front());
                tasks_.pop_front();
            }
            job(); // packaged_task captures and stores user exceptions.
        }
    }
public:
    explicit JobPool(std::size_t worker_count) {
        require(worker_count > 0, "at least one worker");
        try {
            for (std::size_t i = 0; i < worker_count; ++i)
                workers_.emplace_back([this] { worker(); });
        } catch (...) {
            { std::scoped_lock lock(mutex_); stopping_ = true; }
            changed_.notify_all();
            throw; // jthreads join while members unwind.
        }
    }
    JobPool(const JobPool&) = delete;
    JobPool& operator=(const JobPool&) = delete;
    ~JobPool() {
        { std::scoped_lock lock(mutex_); stopping_ = true; }
        changed_.notify_all();
        // Workers drain queued tasks. jthread destructors join the threads.
    }
    template<class F>
    auto submit(F&& callable) -> std::future<std::invoke_result_t<F&>> {
        using Result = std::invoke_result_t<F&>;
        auto task = std::make_shared<std::packaged_task<Result()>>(
            std::forward<F>(callable));
        auto result = task->get_future();
        {
            std::scoped_lock lock(mutex_);
            if (stopping_) throw std::logic_error("pool already stopping");
            tasks_.emplace_back([task] { (*task)(); });
        }
        changed_.notify_one();
        return result;
    }
};

// Fixed task boundaries and fixed collection order make this particular
// integer-valued reduction reproducible. General floating-point reduction
// can differ with changed partitions and reassociation choices.
double deterministic_chunk_sum(JobPool& pool, std::span<const double> input,
                               std::size_t grain) {
    require(grain > 0, "grain must be positive");
    std::vector<std::future<double>> futures;
    for (std::size_t from = 0; from < input.size(); from += grain) {
        const auto count = std::min(grain, input.size() - from);
        const auto chunk = input.subspan(from, count);
        futures.push_back(pool.submit([chunk] {
            double subtotal = 0;
            for (const double value : chunk) subtotal += value;
            return subtotal;
        }));
    }
    double total = 0;
    for (auto& future : futures) total += future.get(); // Fixed order.
    return total;
}

void test_job_pool_and_dependencies() {
    JobPool pool{3};
    std::vector<double> bytes(10000);
    for (std::size_t i = 0; i < bytes.size(); ++i)
        bytes[i] = static_cast<double>(i % 4);
    const double reference = std::accumulate(bytes.begin(), bytes.end(), 0.0);
    require(reference == 15000, "serial oracle");
    for (std::size_t grain : {std::size_t{1}, std::size_t{127}, std::size_t{4096}})
        require(deterministic_chunk_sum(pool, bytes, grain) == reference,
                "parallel result must match exact integer-valued oracle");

    auto error = pool.submit([]() -> int {
        throw std::runtime_error("task failure");
    });
    bool propagated = false;
    try { (void)error.get(); }
    catch (const std::runtime_error&) { propagated = true; }
    require(propagated, "packaged_task propagates task error");

    auto one = pool.submit([] { return 7; });
    auto two = pool.submit([&] { return one.get() + 5; });
    require(two.get() == 12, "explicit future dependency");
}

void test_amdahl() {
    const double serial_fraction = 0.20, processors = 4;
    const double speedup =
        1.0 / (serial_fraction + (1.0 - serial_fraction) / processors);
    require(std::abs(speedup - 2.5) < 1e-12, "Amdahl speedup");
}

// The optional benchmark has NO pass/fail timing threshold. Noise, target
// ISA, cache state, CPU frequency, allocator and compiler all matter.
void optional_benchmark() {
    constexpr std::size_t n = 1000000;
    std::vector<ParticleAoS> aos(n);
    ParticleSoA soa(n);
    for (std::size_t i = 0; i < n; ++i) {
        aos[i].x = soa.x[i] = static_cast<float>(i % 17);
        aos[i].mass = soa.mass[i] = 1.0F;
    }
    std::atomic<float> sink{0};
    auto measure = [&](auto&& compute) {
        std::array<double, 7> milliseconds{};
        for (int run = 0; run < 7; ++run) {
            auto start = std::chrono::steady_clock::now();
            const float result = compute();
            const auto end = std::chrono::steady_clock::now();
            sink.store(result, std::memory_order_relaxed);
            milliseconds[static_cast<std::size_t>(run)] =
                std::chrono::duration<double, std::milli>(end-start).count();
        }
        std::sort(milliseconds.begin(), milliseconds.end());
        return milliseconds[3];
    };
    auto aos_time = measure([&] {
        float result = 0;
        for (const auto& p : aos) result += p.x * p.mass;
        return result;
    });
    auto soa_time = measure([&] {
        float result = 0;
        for (std::size_t i = 0; i < n; ++i)
            result += soa.x[i] * soa.mass[i];
        return result;
    });
    std::cout << "bench_median_ms, AoS=" << aos_time << ", SoA=" << soa_time
              << ", result=" << sink.load(std::memory_order_relaxed) << "\n";
}
} // namespace allmight::phase4

int main(int argc, char**) {
    using namespace allmight::phase4;
    try {
        test_layout_and_dense_iteration();
        test_padding_and_atomic_increment();
        test_release_acquire_message();
        test_allocators_and_frame_arena();
        test_deferred_retirement();
        test_job_pool_and_dependencies();
        test_amdahl();
        if (argc > 1) optional_benchmark();
        std::cout << "phase4_cpu_systems_lab: all invariants passed\n";
    } catch (const std::exception& error) {
        std::cerr << "phase4_cpu_systems_lab: FAIL: " << error.what() << "\n";
        return 1;
    }
    return 0;
}
