#include "asset_catalog.hpp"

#include <array>
#include <concepts>
#include <cstddef>
#include <exception>
#include <forward_list>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace {
using namespace allmight::m018;

// An assert macro disappears under -DNDEBUG; this check never does.
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

struct WorldSpace {};
struct ViewSpace {};
struct Meters {};
struct Seconds {};

// Deliberately restrictive: user-defined dual/autodiff scalars need a
// different concept that states and tests their required algebra.
template<class T>
concept RealScalar = std::floating_point<T>;

template<RealScalar T, std::size_t N, class Space>
struct Vector {
    using scalar_type = T;
    using space_type = Space;
    static constexpr std::size_t extent = N;
    std::array<T, N> coordinates{};
    constexpr T& operator[](std::size_t index) { return coordinates[index]; }
    constexpr T operator[](std::size_t index) const { return coordinates[index]; }
};
template<RealScalar T, std::size_t N, class Space>
constexpr Vector<T, N, Space> operator+(
    const Vector<T, N, Space>& a, const Vector<T, N, Space>& b) {
    Vector<T, N, Space> out{};
    for (std::size_t i = 0; i < N; ++i) out[i] = a[i] + b[i];
    return out;
}

template<RealScalar T, class Unit> struct Quantity { T value{}; };
template<RealScalar T, class Unit>
constexpr Quantity<T, Unit> operator+(
    Quantity<T, Unit> a, Quantity<T, Unit> b) {
    return {a.value + b.value};
}

// A small expression tree postpones materialization. Its references
// may dangle when operands or temporary child nodes are destroyed.
template<class E>
concept VectorExpression = requires(const E& expr, std::size_t i) {
    typename E::scalar_type;
    typename E::space_type;
    requires RealScalar<typename E::scalar_type>;
    { E::extent } -> std::convertible_to<std::size_t>;
    { expr[i] } -> std::same_as<typename E::scalar_type>;
};
template<class L, class R>
concept CompatibleExpressions = VectorExpression<L> && VectorExpression<R>
    && std::same_as<typename L::scalar_type, typename R::scalar_type>
    && std::same_as<typename L::space_type, typename R::space_type>
    && (L::extent == R::extent);

template<class L, class R> requires CompatibleExpressions<L, R>
struct SumExpr {
    using scalar_type = typename L::scalar_type;
    using space_type = typename L::space_type;
    static constexpr std::size_t extent = L::extent;
    const L& left;
    const R& right;
    constexpr auto operator[](std::size_t i) const {
        return left[i] + right[i];
    }
};
template<class L, class R> requires CompatibleExpressions<L, R>
constexpr SumExpr<L, R> lazy_add(const L& a, const R& b) {
    return {a, b};
}
template<RealScalar T, std::size_t N, class Space, VectorExpression Expr>
    requires std::same_as<T, typename Expr::scalar_type>
        && std::same_as<Space, typename Expr::space_type>
        && (N == Expr::extent)
constexpr Vector<T, N, Space> materialize(const Expr& expr) {
    Vector<T, N, Space> out{};
    for (std::size_t i = 0; i < N; ++i) out[i] = expr[i];
    return out;
}

// Runtime dimensions are not encoded in the type; check before access.
template<RealScalar T>
T dot_dynamic(std::span<const T> a, std::span<const T> b) {
    if (a.size() != b.size())
        throw std::invalid_argument("mismatched dimensions");
    T total{};
    for (std::size_t i = 0; i < a.size(); ++i)
        total += a[i] * b[i];
    return total;
}

// Intrusive storage keeps next/link state inside the client's Task object.
// The stack does not own Tasks. Linked tasks must outlive the stack and
// remain at the same address; destruction of the stack unlinks survivors.
struct Task {
    std::string name;
    Task* next{};
    bool linked{};
};
class IntrusiveTaskStack {
    Task* head_{};
public:
    IntrusiveTaskStack() = default;
    IntrusiveTaskStack(const IntrusiveTaskStack&) = delete;
    IntrusiveTaskStack& operator=(const IntrusiveTaskStack&) = delete;
    IntrusiveTaskStack(IntrusiveTaskStack&&) = delete;
    IntrusiveTaskStack& operator=(IntrusiveTaskStack&&) = delete;
    ~IntrusiveTaskStack() {
        while (pop()) {}
    }
    void push(Task& task) {
        if (task.linked) throw std::logic_error("task already linked");
        task.next = head_;
        task.linked = true;
        head_ = &task;
    }
    [[nodiscard]] Task* pop() noexcept {
        Task* task = head_;
        if (!task) return nullptr;
        head_ = task->next;
        task->next = nullptr;
        task->linked = false;
        return task;
    }
};

template<class L, class R>
concept CanLazyAdd = requires(const L& a, const R& b) { lazy_add(a, b); };
template<class T, std::size_t N, class Space, class Expr>
concept CanMaterialize = requires(const Expr& expr) {
    materialize<T, N, Space>(expr);
};

void test_handles_and_catalog() {
    AssetCatalog catalog;
    AssetCatalog another_catalog;

    const auto previous = catalog.insert({"teapot", 128});
    check(catalog.live_count() == 1, "live count");
    auto owned_snapshot = catalog.lookup(previous);
    check(owned_snapshot && owned_snapshot->name == "teapot", "initial lookup");

    // Grow the slot vector many times; the logical handle must survive
    // reallocations even though internal Slot addresses may change.
    for (unsigned i = 0; i < 128; ++i) {
        const auto extra = catalog.insert({"extra-" + std::to_string(i), 3});
        check(extra.catalog_id == previous.catalog_id, "stable catalog owner");
    }
    check(catalog.lookup(previous)->name == "teapot",
          "logical handle survives vector growth");
    check(catalog.erase(previous), "first erase");
    check(!catalog.lookup(previous), "stale generation rejected");
    check(owned_snapshot->name == "teapot", "owned copy survives erase");

    const auto replacement = catalog.insert({"terrain", 4096});
    check(replacement.slot == previous.slot, "slot reused");
    check(replacement.generation != previous.generation, "generation advanced");
    check(catalog.lookup(replacement)->vertex_count == 4096,
          "new content belongs to new generation");

    const auto foreign = another_catalog.insert({"other", 7});
    check(!catalog.lookup(foreign), "foreign catalog identity rejected");
    check(!another_catalog.lookup(replacement), "foreign identity rejected");

    AssetCatalog moved = std::move(catalog);
    check(moved.lookup(replacement).has_value(), "moved pool preserves identity");
    bool rejected = false;
    try { (void)catalog.live_count(); }
    catch (const std::logic_error&) { rejected = true; }
    check(rejected, "moved-from catalog rejects method call");
}

void test_generic_math_and_lifetimes() {
    using W3 = Vector<float, 3, WorldSpace>;
    using V3 = Vector<float, 3, ViewSpace>;
    using W2 = Vector<float, 2, WorldSpace>;
    using DW3 = Vector<double, 3, WorldSpace>;
    static_assert(!std::same_as<W3, V3>);
    static_assert(CanLazyAdd<W3, W3>);
    static_assert(!CanLazyAdd<W3, V3>);
    static_assert(!CanLazyAdd<W3, W2>);
    static_assert(!CanLazyAdd<W3, DW3>);
    using Sum = decltype(lazy_add(std::declval<const W3&>(),
                                 std::declval<const W3&>()));
    static_assert(CanMaterialize<float, 3, WorldSpace, Sum>);
    static_assert(!CanMaterialize<float, 3, ViewSpace, Sum>);
    static_assert(!CanMaterialize<float, 2, WorldSpace, Sum>);
    static_assert(!CanMaterialize<double, 3, WorldSpace, Sum>);
    using NestedSum = decltype(lazy_add(std::declval<const Sum&>(),
                                       std::declval<const W3&>()));
    static_assert(CanMaterialize<float, 3, WorldSpace, NestedSum>);
    static_assert(!CanLazyAdd<NestedSum, V3>);
    static_assert(!std::same_as<MeshHandle, TextureHandle>);
    static_assert(!std::same_as<Quantity<float, Meters>, Quantity<float, Seconds>>);
    static_assert(sizeof(W3) == 3 * sizeof(float));
    static_assert(RealScalar<double> && !RealScalar<int>);

    constexpr W3 a{{1, 2, 3}}, b{{4, 5, 6}}, c{{7, 8, 9}};
    constexpr auto eager = a + b + c;
    // Reference-bearing expression objects point to automatic locals;
    // do not require their addresses to be core constant expressions.
    const auto inner = lazy_add(a, b);
    const auto lazy = materialize<float, 3, WorldSpace>(lazy_add(inner, c));
    check(eager.coordinates == lazy.coordinates, "lazy and eager agree");
    static_assert(eager.coordinates == std::array<float, 3>{12, 15, 18});

    const std::array<double, 3> left{1, 2, 3}, right{4, 5, 6};
    const std::array<double, 2> short_right{3, 4};
    check(dot_dynamic<double>(left, right) == 32, "dynamic dot result");
    bool mismatch = false;
    try { (void)dot_dynamic<double>(left, short_right); }
    catch (const std::invalid_argument&) { mismatch = true; }
    check(mismatch, "dynamic length mismatch rejected");
}

void test_intrusive_versus_owning() {
    static_assert(!std::is_copy_constructible_v<IntrusiveTaskStack>);
    static_assert(!std::is_copy_assignable_v<IntrusiveTaskStack>);
    static_assert(!std::is_move_constructible_v<IntrusiveTaskStack>);
    static_assert(!std::is_move_assignable_v<IntrusiveTaskStack>);
    Task upload{"upload"}, render{"render"};
    IntrusiveTaskStack queue;
    queue.push(upload);
    queue.push(render);

    bool duplicate = false;
    try { queue.push(upload); }
    catch (const std::logic_error&) { duplicate = true; }
    check(duplicate, "cannot link an intrusive node twice");

    check(queue.pop() == &render, "intrusive order");
    check(queue.pop() == &upload, "intrusive next");
    check(queue.pop() == nullptr, "empty stack");
    check(!upload.linked && !render.linked, "unlink clears membership");

    // Nodes precede the nested stack, so they survive its destructor.
    {
        IntrusiveTaskStack temporary;
        temporary.push(upload);
        temporary.push(render);
    }
    check(!upload.linked && !render.linked && !upload.next && !render.next,
          "stack destruction unlinks surviving nodes");
    queue.push(upload);
    check(queue.pop() == &upload, "unlinked node can join another stack");

    // This separately allocated container OWNS string values, unlike the
    // intrusive stack. No performance comparison is inferred.
    std::forward_list<std::string> owned{"upload", "render"};
    check(*owned.begin() == "upload", "non-intrusive owned element");
}
} // namespace

int main() {
    try {
        test_handles_and_catalog();
        test_generic_math_and_lifetimes();
        test_intrusive_versus_owning();
        std::cout << "m018: PIMPL/handles/typed math/lifetimes: PASS\n";
    } catch (const std::exception& e) {
        std::cerr << "m018: FAIL: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
