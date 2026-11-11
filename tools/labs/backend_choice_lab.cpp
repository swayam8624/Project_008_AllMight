// Phase 1 of AllMight: one engine request, two forms of runtime choice.
// Build with C++20, warnings-as-errors, and ASan/UBSan when available.
#include <cassert>
#include <compare>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <variant>
#include <type_traits>

// A request is one of three known shapes. std::variant stores the active tag
// and constructs/destructs exactly the active alternative.
struct Clear { float red, green, blue; };
struct DrawMesh { std::uint32_t mesh_id, material_id; };
struct Upload { std::uint64_t byte_count; };
using Request = std::variant<Clear, DrawMesh, Upload>;

struct Counts { int clears=0, draws=0, uploads=0; std::uint64_t bytes=0; };

// An interface means the caller knows what a backend promises, not which
// implementation the application selected at runtime.
class Backend {
public:
    virtual ~Backend() = default; // critical for deleting via Backend*
    virtual void submit(const Request& request) = 0;
    virtual std::string_view name() const noexcept = 0;
};

class CountingBackend final : public Backend {
public:
    CountingBackend(Counts& counts, int& destroyed)
        : counts_(counts), destroyed_(destroyed) {}
    ~CountingBackend() override { ++destroyed_; }

    void submit(const Request& request) override {
        std::visit([this](const auto& operation) {
            using T = std::decay_t<decltype(operation)>;
            if constexpr (std::is_same_v<T, Clear>) {
                ++counts_.clears;
            } else if constexpr (std::is_same_v<T, DrawMesh>) {
                ++counts_.draws;
            } else {
                static_assert(std::is_same_v<T, Upload>);
                ++counts_.uploads;
                counts_.bytes += operation.byte_count;
            }
        }, request);
    }

    std::string_view name() const noexcept override { return "counting"; }
private:
    Counts& counts_; // borrowed: the caller keeps this alive during backend use
    int& destroyed_; // borrowed: test observes the derived destructor
};

class NullBackend final : public Backend {
public:
    void submit(const Request&) override {}
    std::string_view name() const noexcept override { return "null"; }
};

// An asset identity compares by both slot and generation. The compiler can
// synthesize equality and a strong (total) ordering for these integer fields.
struct AssetId {
    std::uint32_t slot;
    std::uint32_t generation;
    auto operator<=>(const AssetId&) const = default;
};

// Nontrivial inherited state can be sliced by copying only its base subobject.
struct NamedObject {
    std::string_view label{"base"};
    virtual ~NamedObject() = default;
    virtual std::string_view kind() const noexcept { return "base"; }
};
struct MeshObject : NamedObject {
    MeshObject() { label = "mesh"; }
    std::string_view kind() const noexcept override { return "mesh"; }
};

// Virtual inheritance creates one shared Root subobject even with two routes.
struct Root { int identifier = 42; };
struct Left : virtual Root {};
struct Right : virtual Root {};
struct Diamond : Left, Right {};

// Default arguments bind using the static type at the call site, whereas
// the selected virtual-function body depends on the dynamic type.
struct DispatchBase {
    virtual ~DispatchBase() = default;
    virtual int reserve(int n = 2) const { return n; }
};
struct DispatchDerived : DispatchBase {
    int reserve(int n = 7) const override { return n * 10; }
};

// Overload resolution selects a viable function before execution. These
// overloads expose the actual static type decision to the test.
constexpr int classify(int) { return 1; }
constexpr int classify(double) { return 2; }

int main() {
    Counts counts;
    int derived_destructor_calls = 0;
    {
        std::unique_ptr<Backend> backend =
            std::make_unique<CountingBackend>(counts, derived_destructor_calls);
        assert(backend->name() == "counting");
        backend->submit(Request{Clear{0.1f, 0.2f, 0.3f}});
        backend->submit(Request{DrawMesh{17u, 4u}});
        backend->submit(Request{Upload{4096u}});
        assert(counts.clears == 1 && counts.draws == 1);
        assert(counts.uploads == 1 && counts.bytes == 4096);

        // dynamic_cast tests dynamic type: null would mean another backend.
        auto* concrete = dynamic_cast<CountingBackend*>(backend.get());
        assert(concrete != nullptr);
        assert(dynamic_cast<NullBackend*>(backend.get()) == nullptr);
    }
    assert(derived_destructor_calls == 1); // virtual deletion reached the derived owner

    Request request = DrawMesh{8u, 2u};
    assert(std::holds_alternative<DrawMesh>(request));
    assert(std::get_if<Upload>(&request) == nullptr);
    request = Upload{128u};
    assert(std::get<Upload>(request).byte_count == 128u);

    const AssetId original{7u, 3u};
    assert((original == AssetId{7u, 3u}));
    assert((original != AssetId{7u, 4u}));
    assert((original <=> AssetId{8u, 1u}) < 0);
    // Floating-point order differs: NaN is *unordered*, not less/equal/greater.
    const double nan = std::numeric_limits<double>::quiet_NaN();
    assert((nan <=> 1.0) == std::partial_ordering::unordered);

    MeshObject mesh;
    NamedObject& borrowed = mesh;
    assert(borrowed.kind() == "mesh"); // virtual call reaches derived override
    NamedObject sliced = mesh;          // makes a *new base object*, losing derived type
    assert(sliced.kind() == "base");

    Diamond diamond;
    Root* via_left = static_cast<Root*>(static_cast<Left*>(&diamond));
    Root* via_right = static_cast<Root*>(static_cast<Right*>(&diamond));
    assert(via_left == via_right && via_left->identifier == 42);

    DispatchDerived derived;
    DispatchBase& base = derived;
    assert(base.reserve() == 20); // base's default 2; derived's body multiplies by 10
    assert(derived.reserve() == 70); // derived's default 7
    assert(classify(3) == 1 && classify(3.0) == 2);

    // A raw union can reuse storage for trivial members, but it provides no
    // automatic active-tag query. Do not read index after writing weight.
    union TrivialCell { std::uint32_t index; float weight; };
    TrivialCell cell{.index=5u};
    assert(cell.index == 5u);
    cell.weight = 0.5f;
    assert(cell.weight == 0.5f);

    std::cout << "backend_choice_lab: all semantic checks passed\n";
}
