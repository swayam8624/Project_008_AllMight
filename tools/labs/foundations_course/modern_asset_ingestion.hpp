#pragma once
// AllMight Book 1 / Pass18 — owned C++23 filesystem/ranges/coroutine ingestion.
// Integrates *real* existing TXR4/MSH1 codec inputs; the receiving lab calls
// the Pass06 generational asset registry. Never expose borrowed file buffers.
//
// This portable reference checks lexical paths, rejects encountered symlinks,
// canonicalizes the existing target and caps reads. std::filesystem + ifstream
// cannot make this security policy race-free against a hostile concurrent
// filesystem writer (TOCTOU). Production sandboxed native I/O must use
// dirfd/openat2/openat O_NOFOLLOW/RESOLVE_BENEATH or platform equivalents.
#include <algorithm>
#include <array>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <expected>
#include <filesystem>
#include <fstream>
#include <functional>
#include <ios>
#include <limits>
#include <mutex>
#include <memory>
#include <stdexcept>
#include <optional>
#include <ranges>
#include <span>
#include <stop_token>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <atomic>
#include <utility>
#include <vector>
#if __has_include(<format>)
#include <format>
#endif
#if __has_include(<print>)
#include <print>
#endif

namespace allmight::foundations::ingestion {
namespace fs = std::filesystem;
using Byte = std::uint8_t;

enum class AssetKind { texture_rgba8, triangle_mesh };
enum class Issue { unsafe_relative_path, symlink, missing_file,
    not_regular, io_failure, too_large, cancelled, duplicate_request, aggregate_budget };
struct Request {std::string relative; AssetKind kind{}; bool enabled{true};};
struct OwnedAsset {
    std::string relative;
    AssetKind kind{};
    std::vector<Byte> bytes;
    std::uint64_t checksum{};
};
using Result=std::expected<OwnedAsset,Issue>;

// A stable, bytewise, deliberately NON-cryptographic fixture fingerprint.
// Never confuse this with authentication or a security hash.
[[nodiscard]] inline std::uint64_t fnv1a(std::span<const Byte> data) noexcept {
    std::uint64_t v=14695981039346656037ULL;
    for(Byte byte:data){v^=byte;v*=1099511628211ULL;}
    return v;
}
[[nodiscard]] inline bool safe_relative(const fs::path& p) {
    if(p.empty() || p.is_absolute() || p.has_root_name() || p.has_root_directory())
        return false;
    for(const auto& part:p)
        if(part.empty() || part=="." || part=="..") return false;
    return true;
}
[[nodiscard]] inline bool beneath(const fs::path& root,const fs::path& target) {
    const auto rel=target.lexically_relative(root);
    return safe_relative(rel);
}

class Root {
public:
    explicit Root(fs::path root,std::size_t cap=1024*1024)
      : cap_(cap) {
        if(cap==0) throw std::invalid_argument("zero max asset bytes");
        std::error_code ec;
        const auto status=fs::symlink_status(root,ec);
        if(ec || fs::is_symlink(status))
            throw std::invalid_argument("root is absent or symlink");
        path_=fs::canonical(root,ec);
        if(ec || !fs::is_directory(path_))
            throw std::invalid_argument("root must be existing directory");
    }
    [[nodiscard]] Result load(const Request& request) const {
        const fs::path relative=fs::path(request.relative);
        if(!safe_relative(relative))
            return std::unexpected(Issue::unsafe_relative_path);
        auto cursor=path_;
        std::error_code ec;
        for(const auto& segment:relative) {
            cursor/=segment;
            const auto status=fs::symlink_status(cursor,ec);
            if(ec) return std::unexpected(Issue::missing_file);
            if(fs::is_symlink(status)) return std::unexpected(Issue::symlink);
        }
        const auto resolved=fs::canonical(cursor,ec);
        if(ec) return std::unexpected(Issue::missing_file);
        if(!beneath(path_,resolved))
            return std::unexpected(Issue::unsafe_relative_path);
        if(!fs::is_regular_file(resolved,ec) || ec)
            return std::unexpected(Issue::not_regular);
        // No trusting a filesystem size for allocation. Read capped chunks,
        // explicitly detecting a limit+1 byte. Real concurrent writes can
        // still change content during this operation, per TOCTOU warning.
        std::ifstream in(resolved,std::ios::binary);
        if(!in) return std::unexpected(Issue::io_failure);
        std::vector<Byte> bytes;
        std::array<char,4096> chunk{};
        while(in) {
            in.read(chunk.data(),static_cast<std::streamsize>(chunk.size()));
            const auto n=in.gcount();
            if(n<0) return std::unexpected(Issue::io_failure);
            const auto count=static_cast<std::size_t>(n);
            if(count>cap_-bytes.size())
                return std::unexpected(Issue::too_large);
            for(std::size_t i=0;i<count;++i)
                bytes.push_back(static_cast<Byte>(static_cast<unsigned char>(chunk[i])));
            if(in.bad()) return std::unexpected(Issue::io_failure);
        }
        if(!in.eof()) return std::unexpected(Issue::io_failure);
        const auto hash=fnv1a(bytes);
        return OwnedAsset{request.relative,request.kind,std::move(bytes),hash};
    }
    [[nodiscard]] const fs::path& path() const noexcept{return path_;}
private:
    fs::path path_;
    std::size_t cap_{};
};

// Coroutine frame owns a copy of ROOT and all request strings. A caller may
// immediately destroy its original vector after make_batch() returns.
// co_yield stores an OWNED result; it never publishes a span into an ifstream
// buffer or an iterator into the coroutine's requests vector.
template<class T> class Generator {
public:
    struct promise_type {
        std::optional<T> current;
        std::exception_ptr failure;
        Generator get_return_object() noexcept {
            return Generator{handle_type::from_promise(*this)};
        }
        std::suspend_always initial_suspend() const noexcept{return {};}
        std::suspend_always final_suspend() const noexcept{return {};}
        std::suspend_always yield_value(T value) noexcept {
            current.emplace(std::move(value));return {};
        }
        void unhandled_exception() noexcept{failure=std::current_exception();}
        void return_void() const noexcept {}
    };
    using handle_type=std::coroutine_handle<promise_type>;
    explicit Generator(handle_type h) noexcept:handle_(h){}
    Generator(const Generator&)=delete;
    Generator& operator=(const Generator&)=delete;
    Generator(Generator&& old) noexcept:handle_(std::exchange(old.handle_,{})){}
    Generator& operator=(Generator&& old) noexcept {
        if(this!=&old){if(handle_)handle_.destroy();handle_=std::exchange(old.handle_,{});}
        return *this;
    }
    ~Generator(){if(handle_)handle_.destroy();}
    [[nodiscard]] std::optional<T> next() {
        if(!handle_ || handle_.done())return std::nullopt;
        auto& p=handle_.promise();
        p.current.reset();
        handle_.resume();
        if(handle_.done()) {
            if(p.failure)std::rethrow_exception(p.failure);
            return std::nullopt;
        }
        if(!p.current)throw std::logic_error("yield contract violated");
        return std::move(p.current);
    }
private:
    handle_type handle_{};
};

[[nodiscard]] inline Generator<Result> make_batch(
    Root root,std::vector<Request> requests,std::stop_token stop) {
    // std::ranges algorithm orders OWNED requests; a view is used only
    // while its underlying vector remains alive inside the coroutine frame.
    std::ranges::sort(requests,{},&Request::relative);
    std::string previous;
    bool has_previous=false;
    for(const auto& request : requests | std::views::filter(
            [](const Request& x){return x.enabled;})) {
        if(stop.stop_requested()) {
            co_yield std::unexpected(Issue::cancelled);
            co_return;
        }
        if(has_previous && previous==request.relative) {
            co_yield std::unexpected(Issue::duplicate_request);
            continue;
        }
        previous=request.relative;
        has_previous=true;
        co_yield root.load(request);
    }
}

// Real host concurrency: a std::jthread resumes the coroutine on ONE worker
// and transfers OWNED results under a mutex. A coroutine is not automatically
// a thread. No frame is resumed concurrently. Requesting stop is cooperative.
class WorkerBatch {
    struct State {
        std::mutex mutex;
        std::vector<Result> results;
        std::exception_ptr failure;
    };
public:
    WorkerBatch(Root root,std::vector<Request> requests,
                std::size_t aggregate_budget=8U*1024U*1024U)
      : state_(std::make_shared<State>()),
        worker_([state=state_,root=std::move(root),requests=std::move(requests),
                 aggregate_budget](std::stop_token stop) mutable {
            try {
                auto gen=make_batch(std::move(root),std::move(requests),stop);
                std::size_t accumulated=0;
                while(auto outcome=gen.next()) {
                    // Budget each owned successful result before retaining
                    // it; never keep an unlimited quantity of decoded bytes.
                    if(outcome->has_value()) {
                        const auto n=outcome->value().bytes.size();
                        if(n>aggregate_budget-accumulated) {
                            std::lock_guard lock(state->mutex);
                            state->results.push_back(
                                std::unexpected(Issue::aggregate_budget));
                            break;
                        }
                        accumulated+=n;
                    }
                    std::lock_guard lock(state->mutex);
                    state->results.push_back(std::move(*outcome));
                    if(stop.stop_requested()) break;
                }
            } catch(...) {
                // An exception must NEVER escape a std::jthread function:
                // doing so would invoke std::terminate for the process.
                std::lock_guard lock(state->mutex);
                state->failure=std::current_exception();
            }
        }) {
        if(aggregate_budget==0) {
            worker_.request_stop();
            worker_.join();
            throw std::invalid_argument("zero aggregate byte budget");
        }
    }
    WorkerBatch(const WorkerBatch&)=delete;
    WorkerBatch& operator=(const WorkerBatch&)=delete;
    void request_stop() noexcept {worker_.request_stop();}
    [[nodiscard]] std::vector<Result> finish() {
        if(worker_.joinable())worker_.join();
        std::lock_guard lock(state_->mutex);
        if(state_->failure) std::rethrow_exception(state_->failure);
        return std::move(state_->results);
    }
private:
    std::shared_ptr<State> state_;
    std::jthread worker_;
};


// An explicit co_await suspension point demonstrates that a coroutine is a
// frame with a live lifetime, NOT a magic background thread. Its state is
// strongly owned, so resuming after the caller releases its copy is safe.
// Cancellation after suspension must prevent the publication side effect.
class ManualTask {
public:
    struct promise_type {
        ManualTask get_return_object() noexcept {
            return ManualTask{handle_type::from_promise(*this)};
        }
        std::suspend_always initial_suspend() const noexcept {return {};}
        std::suspend_always final_suspend() const noexcept {return {};}
        void return_void() const noexcept {}
        void unhandled_exception() noexcept {failure=std::current_exception();}
        std::exception_ptr failure;
    };
    using handle_type=std::coroutine_handle<promise_type>;
    explicit ManualTask(handle_type h) noexcept:handle_(h){}
    ManualTask(const ManualTask&)=delete;
    ManualTask& operator=(const ManualTask&)=delete;
    ManualTask(ManualTask&& other) noexcept:handle_(std::exchange(other.handle_,{})){}
    ManualTask& operator=(ManualTask&& other) noexcept {
        if(this!=&other){if(handle_)handle_.destroy();handle_=std::exchange(other.handle_,{});}
        return *this;
    }
    ~ManualTask(){if(handle_)handle_.destroy();}
    [[nodiscard]] bool resume() {
        if(!handle_ || handle_.done()) return false;
        handle_.resume();
        if(handle_.done() && handle_.promise().failure)
            std::rethrow_exception(handle_.promise().failure);
        return !handle_.done();
    }
private:
    handle_type handle_{};
};
[[nodiscard]] inline ManualTask publish_after_checkpoint(
    std::shared_ptr<std::atomic<int>> output,std::stop_token stop,int value) {
    co_await std::suspend_always{};
    if(!stop.stop_requested())output->store(value,std::memory_order_release);
}

// C++23 library availability differs between AppleClang/libc++ and GCC;
// the fallback keeps the *whole lab* buildable on incomplete vendor libs.
// The feature-gated primary path remains actual type-safe std::format.
[[nodiscard]] inline std::string diagnostic(Issue issue,std::string_view file){
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
    return std::format("asset '{}' rejected with code {}",file,static_cast<int>(issue));
#else
    return "asset '"+std::string(file)+"' rejected with code "+
        std::to_string(static_cast<int>(issue));
#endif
}
inline void announce_success(std::size_t count){
#if defined(__cpp_lib_print) && __cpp_lib_print >= 202207L
    std::println("AllMight C++23 file/range/coroutine batch: {} accepted assets",count);
#else
    (void)count;
#endif
}
} // namespace allmight::foundations::ingestion
