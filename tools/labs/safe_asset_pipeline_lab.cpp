// AllMight M012 / Parts 181-200: one resource-loading transaction, several callbacks.
// Standalone C++20 laboratory. No Vulkan/Metal work or external repository code.
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace allmight {

void verify(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message); // tests work even with NDEBUG
}

// A byte count is not an asset index or a Boolean. Explicit conversions keep
// accidental integer units out of the API while retaining a cheap value type.
struct ByteCount {
    std::size_t value;
    explicit constexpr ByteCount(std::size_t count) noexcept : value(count) {}
    explicit constexpr operator std::size_t() const noexcept { return value; }
};
static_assert(!std::is_convertible_v<std::size_t, ByteCount>);
static_assert(!std::is_convertible_v<ByteCount, std::size_t>);

struct AssetId { std::size_t index; };
enum class UploadError { EmptyPayload, SimulatedFailure, ResourceUnavailable, OutOfMemory };
using UploadResult = std::variant<AssetId, UploadError>;

// This is a deterministic fake native device, not a GPU emulator. Its fixed
// table lets tests count acquired/released resource identities without malloc.
class FakeDevice {
    std::array<bool, 32> active_{};
    std::size_t next_ = 1;
    std::size_t acquired_ = 0;
    std::size_t released_ = 0;
public:
    std::size_t acquire(ByteCount bytes) {
        if (bytes.value == 0 || next_ >= active_.size())
            throw std::runtime_error("resource unavailable");
        const std::size_t handle = next_++;
        active_[handle] = true;
        ++acquired_;
        return handle;
    }
    void release(std::size_t handle) noexcept {
        // This is an internal invariant, not validation of untrusted input.
        if (handle == 0 || handle >= active_.size() || !active_[handle])
            std::terminate();
        active_[handle] = false;
        ++released_;
    }
    bool is_active(std::size_t handle) const noexcept {
        return handle < active_.size() && active_[handle];
    }
    std::size_t acquired() const noexcept { return acquired_; }
    std::size_t released() const noexcept { return released_; }
    std::size_t live() const noexcept { return acquired_ - released_; }
};

// The constructor acquires exactly one resource; destruction returns it.
// Moving transfers the release duty, not the native resource itself.
// The borrowed FakeDevice must outlive all its Resource objects.
class Resource {
    FakeDevice* device_ = nullptr;
    std::size_t handle_ = 0;
public:
    Resource(FakeDevice& device, ByteCount bytes)
        : device_(&device), handle_(device.acquire(bytes)) {}
    ~Resource() noexcept { reset(); }
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    Resource(Resource&& other) noexcept
        : device_(std::exchange(other.device_, nullptr)),
          handle_(std::exchange(other.handle_, 0)) {}
    Resource& operator=(Resource&& other) noexcept {
        if (this != &other) {
            reset();
            device_ = std::exchange(other.device_, nullptr);
            handle_ = std::exchange(other.handle_, 0);
        }
        return *this;
    }
    void reset() noexcept {
        if (device_ != nullptr) {
            device_->release(handle_);
            device_ = nullptr;
            handle_ = 0;
        }
    }
    std::size_t handle() const noexcept { return handle_; }
};
static_assert(std::is_nothrow_move_constructible_v<Resource>);
static_assert(std::is_nothrow_move_assignable_v<Resource>);
static_assert(!std::is_copy_constructible_v<Resource>);

struct AssetRecord {
    ByteCount size;
    Resource native;
    std::vector<std::byte> source;
    std::string label;
};
static_assert(std::is_nothrow_move_constructible_v<AssetRecord>);

// A new asset remains private until every preparatory operation succeeds.
// Publication is the vector insertion; failed preparation releases the native
// owner and does not change previously published records.
class AssetStore {
    FakeDevice& device_;
    std::vector<AssetRecord> records_;
public:
    explicit AssetStore(FakeDevice& device) : device_(device) {}
    std::size_t size() const noexcept { return records_.size(); }
    const AssetRecord& at(std::size_t i) const { return records_.at(i); }

    UploadResult upload(std::string label, std::vector<std::byte> payload,
                        bool fail_after_acquire = false) {
        if (payload.empty()) return UploadError::EmptyPayload;
        try {
            // Work is held in local owning values. The old records are intact.
            Resource pending(device_, ByteCount{payload.size()});
            if (fail_after_acquire)
                throw std::runtime_error("injected failure after allocation");
            AssetRecord ready{ByteCount{payload.size()}, std::move(pending),
                              std::move(payload), std::move(label)};
            // A vector growth failure preserves the old vector: AssetRecord's
            // move constructor is nonthrowing, as checked above.
            records_.push_back(std::move(ready));
            return AssetId{records_.size() - 1};
        } catch (const std::bad_alloc&) {
            return UploadError::OutOfMemory;
        } catch (const std::runtime_error& e) {
            // In a real engine, use a typed internal error instead of string
            // matching; this simulation has only two runtime-error sources.
            if (fail_after_acquire) return UploadError::SimulatedFailure;
            (void)e;
            return UploadError::ResourceUnavailable;
        }
    }
};

// A normal function pointer has no hidden closure state. A separate context
// pointer is borrowed only for the synchronous duration of the call.
using RawReport = void (*)(void*, ByteCount);
void report_to_counter(void* context, ByteCount bytes) {
    auto& sum = *static_cast<std::size_t*>(context); // valid only for correct context
    sum += bytes.value;
}

struct Reporter {
    std::size_t bytes = 0;
    void report(ByteCount amount) { bytes += amount.value; }
};

// A callable object can have stored state; unlike a function pointer, it can
// carry references or owned objects. Their lifetime contract is still explicit.
struct AddBytes {
    std::size_t* destination;
    void operator()(ByteCount amount) const { *destination += amount.value; }
};

struct WireHeader {
    std::uint32_t marker;
    std::uint16_t version;
};
static_assert(std::is_standard_layout_v<WireHeader>);
static_assert(std::is_trivially_copyable_v<WireHeader>);
// Even with those traits, padding/endian rules are not a portable wire format.
void append_u32_le(std::vector<std::uint8_t>& wire, std::uint32_t value) {
    for (unsigned shift : {0u, 8u, 16u, 24u})
        wire.push_back(static_cast<std::uint8_t>((value >> shift) & 0xffu));
}
void append_u16_le(std::vector<std::uint8_t>& wire, std::uint16_t value) {
    for (unsigned shift : {0u, 8u})
        wire.push_back(static_cast<std::uint8_t>((value >> shift) & 0xffu));
}

void run() {
    FakeDevice device;
    {
        AssetStore assets(device);
        verify(assets.size() == 0, "new store is empty");
        verify(std::get<UploadError>(assets.upload("empty", {})) == UploadError::EmptyPayload,
               "reject empty input as data error");
        verify(device.acquired() == 0, "invalid input must not allocate");

        std::vector<std::byte> first(4, std::byte{0x3a});
        auto success = assets.upload("mesh", first);
        verify(std::holds_alternative<AssetId>(success), "success publishes an id");
        verify(assets.size() == 1 && assets.at(0).label == "mesh", "publish once");
        const std::size_t held = assets.at(0).native.handle();
        verify(device.is_active(held) && device.live() == 1, "new asset is owned");

        auto failed = assets.upload("broken", first, true);
        verify(std::get<UploadError>(failed) == UploadError::SimulatedFailure,
               "translate injected exception to a typed error");
        verify(assets.size() == 1 && assets.at(0).native.handle() == held,
               "strong guarantee: old asset and handle unchanged");
        verify(device.acquired() == 2 && device.released() == 1 && device.live() == 1,
               "failed newly acquired resource is released exactly once");

        auto second = assets.upload("texture", std::vector<std::byte>(9, std::byte{0x55}));
        verify(std::holds_alternative<AssetId>(second), "publish after failure");
        verify(assets.size() == 2 && device.live() == 2, "both assets alive");
        verify(assets.at(0).native.handle() == held, "growing vector preserves resource identity");
    }
    verify(device.live() == 0 && device.acquired() == device.released(),
           "end of store lifetime releases every native owner once");

    std::size_t total = 0;
    RawReport raw = &report_to_counter;
    raw(&total, ByteCount{5});
    Reporter reporter;
    void (Reporter::*member)(ByteCount) = &Reporter::report;
    (reporter.*member)(ByteCount{7});
    AddBytes function_object{&total};
    function_object(ByteCount{11});
    verify(total == 16 && reporter.bytes == 7, "function pointer, member pointer, functor");

    std::size_t changing = 10;
    auto snapshot = [captured = changing] { return captured; };
    auto borrowed = [&changing] { return changing; }; // safe only while changing lives
    changing = 20;
    verify(snapshot() == 10 && borrowed() == 20, "capture by value vs reference");

    std::function<void()> deferred;
    std::weak_ptr<int> observed;
    {
        auto owned = std::make_shared<int>(19);
        observed = owned;
        deferred = [owned, &total] { total += static_cast<std::size_t>(*owned); };
    }
    verify(!observed.expired(), "owning capture survives original scope");
    deferred();
    verify(total == 35, "type-erased callback runs after capture's originating scope");
    deferred = {};
    verify(observed.expired(), "releasing callback releases its ownership");
    std::function<void()> missing;
    bool empty_function_threw = false;
    try { missing(); } catch (const std::bad_function_call&) { empty_function_threw = true; }
    verify(empty_function_threw, "empty std::function reports bad_function_call");

    static_assert(sizeof(float) == sizeof(std::uint32_t));
    static_assert(std::numeric_limits<float>::is_iec559);
    const std::uint32_t bits = std::bit_cast<std::uint32_t>(1.0f);
    verify(bits == 0x3f800000u, "IEEE binary32 representation of 1.0");
    verify(std::bit_cast<float>(bits) == 1.0f, "bit_cast roundtrip");
    std::vector<std::uint8_t> wire;
    append_u32_le(wire, 0x12345678u);
    append_u16_le(wire, 0x0302u);
    verify(wire == (std::vector<std::uint8_t>{0x78, 0x56, 0x34, 0x12, 0x02, 0x03}),
           "wire encoding is explicit, independent of struct padding or host endian");

    std::cout << "safe_asset_pipeline_lab: all checks passed\n";
}
} // namespace allmight

int main() {
    try { allmight::run(); }
    catch (const std::exception& error) {
        std::cerr << "safe_asset_pipeline_lab failed: " << error.what() << '\n';
        return 1;
    }
}
