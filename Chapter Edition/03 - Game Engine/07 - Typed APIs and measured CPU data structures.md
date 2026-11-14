# 07 — Typed APIs and measured CPU data structures

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/06 - Queries, lazy ranges, allocators, and module boundaries|Previous chapter]] · [[Chapter Edition/03 - Game Engine/08 - Algorithms, application boundaries, and a first playable|Next chapter]]

An asset service now needs an interface that hides implementation without hiding identity or cost. The route then moves from typed boundaries into CPU execution, memory traffic, concurrency, and choosing containers for actual workloads.

**By the end:** Connect strong types and PIMPL to measured CPU costs, then select containers and scheduling structures by their operation patterns.

**Recall question:** Which promises does the public API make, and what measurement would show whether its internal representation meets them?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 9067–9743; source-span SHA256 b7e4a76ceb20782e78e411986c4de6fa7a969f0117a4a2ba87445aa7e273f4a8. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## A registry crosses its last C++ boundary: hidden implementation, typed identity and measured abstraction cost

The previous asset-query service was deliberately constructed across several translation units. A query could be imported through a C++20 module, allocated through a declared memory resource, expressed as a lazy range and made available to a separately compiled consumer. That was useful, but it leaves one particularly dangerous design pressure unresolved. The editor must keep a mesh selected while other assets are loaded and destroyed; graphics code must not mistake a texture ID for a mesh ID or a world-space displacement for a camera-space displacement; plugin clients should not be rebuilt whenever an internal pool representation changes. At the same time, every abstraction placed on the rendering hot path has a cost that we must **observe**, not merely describe as zero.

Follow the concrete operation. A user selects a teapot mesh. The editor holds a handle. The asset catalog replaces its internal vector storage, deletes the original mesh and reuses its slot for terrain. Meanwhile, a separately built rendering component calls a public catalog function, and the transform library calculates a world-space vector. Which values preserve identity? Which operations compile? Which references become invalid? What does a client binary know about internal layout? What will the compiler actually execute?

This is M018, the final Phase 3 mastery chunk and the exact original **Parts 291–300**. It develops a single connected architecture: PIMPL (291); strong typedefs and handles (292); phantom types for units/spaces (293); scalar-constrained generic mathematics (294); compile-time and dynamic dimensions (295); expression-template evaluation and its temporary-lifetime hazards (296); intrusive versus non-intrusive storage (297); measurable, rather than asserted, zero-overhead behavior (298); generated assembly (299); and a reusable container contract that states ownership and invalidation precisely (300). The runnable witness is [[tools/m018/m018_lab.cpp|the standalone lab]], linked through a genuinely separate implementation file in [[tools/m018/asset_catalog.cpp|the asset catalog]] and its stable public [[tools/m018/asset_catalog.hpp|interface]]. The reproducible build and assembly commands are in [[tools/m018/build_m018_lab.sh|the build script]]. These files demonstrate selected semantics; they are not the current Kairo engine.

### Hide representation only after deciding what clients are entitled to know

A client that includes an ordinary class declaration sees the layout of every member, even the private ones. Private access control prevents source-level member access; it does **not** hide the existence, size or alignment contribution of a private vector or hash table. Suppose the editor's public catalog contains a vector of mesh slots. A change to the slot representation can alter the catalog's size or force client translation units to recompile. A public member function may depend on private helper types as well. This is a compilation-dependency problem first, and sometimes a binary-layout problem.

A pointer-to-implementation, or PIMPL, separates the public class layout from its private representation. Our public header contains only the incomplete declaration **struct Impl;** and **std::unique_ptr<Impl> impl_;**. A pointer has a known representation even while the pointed-to class is incomplete. The actual Impl, containing the vector of slots, free list, catalog identity and live count, appears in the separately compiled source file. Changing Impl's vector into a sparse-set-like layout does not, by itself, require changing the public class's data-member layout.

The public declarations deliberately say:

~~~cpp
class AssetCatalog {
public:
    AssetCatalog();
    ~AssetCatalog();
    AssetCatalog(const AssetCatalog&) = delete;
    AssetCatalog& operator=(const AssetCatalog&) = delete;
    AssetCatalog(AssetCatalog&&) noexcept;
    AssetCatalog& operator=(AssetCatalog&&) noexcept;

    MeshHandle insert(MeshInfo mesh);
    bool erase(MeshHandle handle);
    std::optional<MeshInfo> lookup(MeshHandle handle) const;

private:
    struct Impl;                 // Incomplete here.
    std::unique_ptr<Impl> impl_; // Pointer ownership known here.
};
~~~

The destructor is *declared* in the header but *defined* after Impl in the source file. Why? The default deleter used by std::unique_ptr must see a complete implementation type at the point where it deletes that type. The same concern applies to move assignment when an old owned implementation is replaced. The default constructor allocates an Impl. The defaulted move operations transfer pointer ownership. Copying is deleted because duplicating a catalog would otherwise create an ambiguous logical-identity policy: is the new catalog a new owner, or does it share the same handles? A deliberate clone operation would have to specify and implement that policy explicitly.

A subtle point: the catalog's C++ **const** member function does not automatically make everything behind the unique_ptr transitively const. If an implementation receives an ordinary non-const pointer through a const owning wrapper, it may still mutate the pointee. The public contract, not the syntax of the pointer alone, must prohibit mutation where required. A propagated-const wrapper or an explicitly const-correct internal access path is needed when deep constness matters. In this lab the exposed lookup returns an owned copy and the implementation's **find** is const.

PIMPL has a nonzero cost in this design: construction allocates the implementation, method calls follow an additional indirection, and methods implemented out-of-line may not be inlined across a normal independently compiled boundary. Some compilers and link-time optimizers may remove selected costs in closed programs; that does not make the public ABI universally free. ABI stability is also conditional: compiler and standard-library ABI agreements, exported symbol signatures, exception boundaries, object layout, platform data models and the definition of public value types still matter. **PIMPL isolates one particular representation dependency; it does not turn an arbitrary C++ API into a universally stable plugin ABI.** The C ABI bridge introduced in M017 has different, more explicit compatibility requirements and also needs versioning, pointer lifetime and allocation ownership rules.

The natural decision is therefore selective. Put PIMPL at an expensive-to-recompile, relatively infrequently called service boundary. Do not put an individually heap-allocated PIMPL inside each of millions of particles or vertices merely because hiding implementation sounds architecturally clean. The latter would introduce allocation and pointer chasing into a workload whose performance depends on contiguous data and predictable locality.

### A handle is not an address: distinguish kind, slot, generation and owner

The editor now stores **MeshHandle**, not a pointer to a std::vector element. This is a distinct logical-identity promise. A vector reallocation may move every Slot object to a different address, and a deletion may destroy the slot's MeshInfo, but the handle remains a small value whose validity is checked against the current catalog.

The public definition is intentionally simple:

~~~cpp
struct MeshTag {};
struct TextureTag {};

template<class Kind>
struct Handle {
    std::uint64_t catalog_id{};
    std::uint32_t slot{};
    std::uint64_t generation{};

    friend constexpr bool operator==(Handle, Handle) = default;
};

using MeshHandle = Handle<MeshTag>;
using TextureHandle = Handle<TextureTag>;
static_assert(!std::is_same_v<MeshHandle, TextureHandle>);
~~~

A **strong type** prevents accidental mixing of semantically different values that happen to share a machine representation. The tag parameter Kind has no data member in Handle. MeshTag and TextureTag are *phantom types*: they influence type checking without adding a tag object to every handle. The runtime fields that do exist are catalog ID, slot number and generation. Their actual size can include padding; we do not claim a particular packed ABI from the declaration alone.

Why three fields? The type separates resource kinds. The **slot** identifies an index in a particular pool. The **generation** distinguishes different occupants of the same slot through time. The **catalog_id** distinguishes two otherwise separate catalogs that may both contain slot zero at generation one. A typed handle from a different catalog must fail lookup even if its slot and generation accidentally match.

Trace the concrete state transition:

~~~text
Catalog A, slot 0, generation 1: MeshInfo("teapot", 128)
    old = MeshHandle{A, 0, 1}  -> VALID

Erase old:
    MeshInfo destroyed; free-list entry prepared;
    generation becomes 2; live count decreases

Catalog A, slot 0, generation 2: MeshInfo("terrain", 4096)
    replacement = MeshHandle{A, 0, 2} -> VALID
    old         = MeshHandle{A, 0, 1} -> INVALID

Catalog B, slot 0, generation 1: MeshInfo("other", 7)
    MeshHandle{B, 0, 1} supplied to A -> INVALID
~~~

This requires a precise invariant: a live handle resolves if and only if the catalog identity, slot bounds, slot's generation and slot's engaged value all agree. The lab checks every part of that conjunction. Free-list reuse does not resuscitate old handles. When the generation reaches its maximum represented value, the slot is retired rather than wrapping into an old generation. Because generation retirement is a correctness policy, it is not appropriate to silently use unsigned wraparound simply because integer arithmetic makes that convenient.

Now connect this to exception safety. Inserting into a previously freed slot first constructs the new MeshInfo and only afterward removes the slot index from the free list. A failed construction must not leave a slot simultaneously free and live. During erasure, the implementation calls **free_slots.reserve** before destroying the live value, so an allocation failure cannot strand a destroyed slot that was meant to be reusable. Only after preparation succeeds does it reset the value, advance the generation and append the free index. For this particular MeshInfo, the move operations have the non-throwing behavior needed by the demonstrated path. A generic container accepting arbitrarily throwing destructors and moves would need a stronger, separately stated contract.

An owned lookup returns **std::optional<MeshInfo>**, not a reference into the vector. When the editor calls lookup and stores that result, it receives a copy of the mesh name and vertex count. It can retain the copy after the original asset is erased. That safety has a price: copying a long std::string may allocate, and an owned snapshot can become *out of date* without becoming *dangling*. These are different failure modes. The renderer that needs the current live resource must validate a handle at the time of use, rather than treating a cached description as proof that the live resource still exists.

A complete engine may also need an asset epoch across application restarts, device identity, serialization policies and atomic reader/writer publication. The catalog's monotonically allocated process-local ID prevents identity reuse among catalogs served by that counter until exhaustion, at which point construction fails, but it is not a durable cross-process or network identity. Moving a catalog transfers its implementation and preserves its handles; methods on the resulting moved-from catalog deliberately reject use until it is reassigned. A copy operation is not offered. These are explicit API decisions, not implicit guarantees of every PIMPL design.

### Strong mathematical types do not perform mathematical conversions for us

The same distinction between accidental representation equality and semantic identity appears in engine mathematics. A world-space displacement and a view-space displacement may both consist of three floats, but they inhabit different coordinate systems. Adding them directly is not meaningful without a conversion. In the laboratory:

~~~cpp
struct WorldSpace {};
struct ViewSpace {};

template<class T>
concept RealScalar = std::floating_point<T>;

template<RealScalar T, std::size_t N, class Space>
struct Vector {
    using scalar_type = T;
    using space_type = Space;
    static constexpr std::size_t extent = N;
    std::array<T, N> coordinates{};
    constexpr T& operator[](std::size_t i) { return coordinates[i]; }
    constexpr T operator[](std::size_t i) const { return coordinates[i]; }
};

using W3 = Vector<float, 3, WorldSpace>;
using V3 = Vector<float, 3, ViewSpace>;
static_assert(!std::same_as<W3, V3>);
~~~

The scalar type T, dimension N and space marker are independent template parameters. T controls element representation and permitted operations. N is a **non-type template parameter** selecting the compile-time extent. Space makes the coordinate frame part of the type. A Vector<float,3,WorldSpace> stores the three floats directly in an array rather than allocating them through an abstract base class. The observed representation assertion in the lab checks that this simple specific type uses exactly 3 * sizeof(float) bytes for its three components under the tested standard-library/ABI configuration; it does not establish the size of all possible generic vector classes.

A phantom **unit** tag can similarly distinguish meters from seconds. Yet it is essential not to confuse tagging with dimensional analysis. A meaningful unit system should define how multiplication, division, powers and conversions transform dimensions: distance divided by time is velocity; a position minus a position yields a displacement, not another position; degrees and radians may both describe angle but require a declared conversion; a point under an affine transform receives translation, while a direction does not.

Coordinate transformation is also not a tag reinterpretation. Under a linear coordinate change with matrix A, a vector transforms according to the declared map, for example $\mathbf{v}_{\mathrm{view}}=A\mathbf{v}_{\mathrm{world}}$. A point typically needs homogeneous or affine translation as well. Covectors and surface normals do not generally transform by the same matrix as displacement vectors. If coordinates transform by A, dual quantities require the inverse-transpose relationship under the corresponding conventions. These facts were developed in the mathematics volume; putting them into C++ types makes illegal combinations harder to express, but does not replace the derivation or the transformation implementation.

The scalar constraint deserves equal scrutiny. The lab's **std::floating_point** concept intentionally admits built-in float, double and long double, and rejects int. A constraint is more useful than a vague generic promise because it states what operations the code is written for. But it also excludes many useful user-defined scalar systems. Automatic-differentiation dual numbers, fixed-point numbers, intervals and arbitrary-precision types may not satisfy std::floating_point. A more powerful engine mathematics library may define a separate field-like concept containing precisely the operations, identities and numerical policies it requires. Merely permitting an **operator+** in a concept is not proof of a mathematical field, floating-point stability, total ordering or differentiability.

The distinction between static and dynamic dimensions appears when the same math library must operate on a 3D transform and on a runtime-sized optimization problem. For N = 3, the compiler knows the element count and can instantiate fixed-size loops or unroll them when profitable. A dynamic dot product accepts spans whose lengths are only known at runtime; it must compare those lengths before indexing. If the two spans differ, the laboratory throws **std::invalid_argument** rather than silently truncating to the shorter extent. A span is a *borrowed view* and does not extend its storage's lifetime; choosing a dynamic extent does not automatically choose ownership.

For a dot product of size n, the straightforward algorithm is

\[
s=\sum_{i=0}^{n-1}a_i b_i.
\]

It performs n multiplications and n additions in a single pass (counting the first accumulation into zero), with O(n) time and O(1) auxiliary scalar storage. Floating-point addition, however, is not associative, so a compiler's SIMD reduction or reassociation may change rounding. The mathematical formula and the machine's numerical contract must not be casually equated. For large n, error analysis, compensated summation and reduction order may matter more than whether the vector's dimension was a template parameter.

### Expression templates: delaying evaluation can remove an intermediate and create a lifetime bug

Our editor also wants to evaluate a mathematical expression with several operands. An eager implementation of **a + b + c** commonly describes two vector additions: compute a+b, then add c. At the abstract source level, an intermediate vector exists. A capable optimizing compiler may elide it, inline both operations or fuse the loops; the language source alone does not tell us the final memory traffic.

An expression template postpones element evaluation by constructing a small type describing the addition:

The code below includes the restrictions as well as the delayed arithmetic. **VectorExpression** describes an indexable expression with a scalar type, coordinate space and extent. **CompatibleExpressions** asks whether two expressions agree on all three. The requires clauses make those checks a condition for selecting lazy_add and materialize. The next section explains why each check is needed and how the negative tests exercise it.

~~~cpp
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
~~~

**lazy_add(a,b)** does not traverse a or b. Its result stores two references. Calling **`operator[](i)`** computes the corresponding sum on demand. Materialization traverses i = 0,...,N−1 once and writes the final array. This demonstrates an opportunity to avoid explicit intermediate storage, not a guaranteed speedup.

We need to prove two different things: **the expression produces the same intended values** and **all referenced operands are still alive**. A seemingly elegant expression

~~~cpp
auto doomed = lazy_add(lazy_add(a, b), c);
~~~

contains a temporary *inner* SumExpr bound to the outer function's reference parameter. The temporary does not live for the lifetime of **doomed**. If we later evaluate doomed, its saved reference to the inner expression is dangling; the code has undefined behavior. No amount of benchmarking makes undefined behavior into an optimization.

The tested example deliberately gives the inner node a name:

~~~cpp
const auto inner = lazy_add(a, b);
const auto lazy = materialize<float, 3, WorldSpace>(
    lazy_add(inner, c));
~~~

The word **const** here means that we do not modify the named object after initialization. **constexpr** imposes an additional requirement: this particular initializer must be a constant expression. A function declared constexpr can still be called at runtime. Inside the lab function, a, b and c are automatic local objects. The expression node saves references to those particular objects; declaring the saved node constexpr would require an initializer that cannot retain those automatic-local addresses in this way. Giving the node a name fixes its lifetime during this evaluation, but does not make its initializer a constant expression. The complete compiled example therefore uses const for inner and lazy. This is separate from the eager result, which stores its numeric components rather than references and is checked with static_assert.

Here a, b, c and inner remain alive throughout materialization. The resulting three components are 12, 15 and 18, matching eager addition. A mature expression-template library needs an operand-storage policy that distinguishes borrowed lvalues from owned rvalue expression nodes. It also needs rules for aliasing: for a pure componentwise addition, writing output[i] may be safe with exact operand/output aliasing, but shifted spans, matrix products and stencil operations can read locations already overwritten by earlier iterations. A general promise that 'lazy expressions remove all temporaries' is wrong without those constraints.

This design decision connects to numeric reproducibility and SIMD. Fusing loops changes when intermediate values are rounded; using fused multiply-add, vector reductions or fast-math transformations can also change results. Measure both correctness and cost. The compiler may outperform a manually built expression tree on a tiny three-float vector; for a large dynamic vector, a single materialization pass may reduce memory bandwidth. Neither outcome is automatic.

### The delayed path must preserve the same mathematical contract

Suppose the compiler rejects world + view, but accepts lazy_add(world, view) followed by materialize<float,3,WorldSpace>. The numeric components can still add up, yet the output has acquired a coordinate label without any coordinate conversion. We have protected one spelling of addition and left another spelling unchecked. A type distinction is useful only when every entry point respects it.

The complete lab now attaches three pieces of compile-time information to each vector and sum node: scalar_type, space_type and extent. A **type alias** gives an existing type a name inside the class. A **static constexpr extent** records the number of components without storing another field in each vector. A sum node carries the same information as its operands. CompatibleExpressions requires matching scalar types, spaces and extents before lazy_add can construct a node. Materialization checks those three properties again against the requested output. Choosing ViewSpace in the output template argument is therefore a request that must be validated, rather than a conversion operation.

A **requires expression** asks whether an operation is well formed for template parameters, without executing it. The regression checks use that question to test actual operations. They accept WorldSpace-three plus WorldSpace-three; reject WorldSpace plus ViewSpace; reject two components plus three; and reject mixing float and double under this deliberately strict policy. They also reject relabelling or resizing a valid sum during materialization. A nested sum keeps its traits, so adding another node cannot quietly discard the restriction. See the complete [[tools/m018/m018_lab.cpp|constrained vector-expression lab]] for the exact declarations and checks.

These checks do not prove that a reference remains alive. A correctly labelled expression can still borrow a destroyed operand. The small lab continues to borrow its children and evaluate them while all named operands remain alive. A reusable deferred-expression library needs a separate policy for owning temporary child nodes, borrowing stable operands, aliasing and mutation. Compile-time shape compatibility and runtime lifetime validity are different questions; solving one does not solve the other.

### Intrusive storage exposes a different ownership contract

The catalog's vector is non-intrusive in the sense that its free-list links are stored in the catalog's bookkeeping rather than embedded in an independent mesh object. Contrast that with a per-frame task stack in which each Task contains its own **next** pointer and linked flag. The intrusive stack points directly to caller-owned Task objects. Pushing a Task changes its link, and popping it resets the link. No separate list node is allocated by this simple stack.

The advantage is a clear and often low-allocation representation. But the stack does **not** own the Task objects. Destroying or moving a Task while its address remains linked causes the container to retain an invalid pointer. Linking the same node twice can create cycles or corrupt ordering. The laboratory rejects a duplicate link and clears membership on pop, but it cannot automatically prevent an external owner from prematurely destroying a linked Task. An intrusive production container should define unlinking, hook membership, multiple-list participation, move restrictions and object-destruction policy. Thread-safe linkage would add synchronization requirements not represented in the sample.

There is another owner question inside the container itself. If we copy only its head pointer, both stacks point at the same hooks. Start with upload linked in stack A. Copy A to B. Pop through B: upload is now marked unlinked, but A still points at it. Pushing upload into A again can make it point to itself, so two later pops return the same task. Keeping the task alive does not prevent this logical corruption.

The lab therefore deletes stack copy construction, copy assignment, move construction and move assignment. This small container has one stable identity; it does not promise to transfer a chain. Its destructor pops remaining nodes, clearing their next pointers and membership flags without deleting the tasks. That cleanup has a strict precondition: every linked task must still exist at the same address when the stack is destroyed. Declare caller-owned tasks before a stack that borrows them, or explicitly unlink them before their owner destroys or relocates them. A cleanup loop cannot repair a node that is already gone.

The regression declares two tasks outside a nested stack scope, links them, exits that scope, and checks that both hooks are clear. It then links a surviving task into another stack and pops it successfully. Compile-time checks also verify that copying or moving the stack is unavailable. Thus the example tests both the permitted operation and the forbidden operation that previously defeated its membership flag.

A non-intrusive **std::forward_list<std::string>** instead owns its string elements inside separately managed nodes. This makes the container's ownership obligations explicit, although it can allocate nodes and add pointer indirection. The comparison is not simply 'intrusive fast, non-intrusive slow': data density, iteration order, size, allocator strategy, mutation frequency, object lifetime and locality determine the actual outcome. An array-based queue may outperform both for sequential workloads.

### The zero-overhead principle becomes a falsifiable experiment

We now have three different cost categories in one API. Typed handles and phantom types aim to enforce semantic distinctions at compile time while storing ordinary scalar fields at runtime. An inline generic addition can often compile to ordinary arithmetic without virtual dispatch. PIMPL intentionally allocates and dereferences an implementation pointer. An intrusive collection may avoid node allocations but requires strict external lifetime discipline.

Calling all of these 'zero-cost abstractions' would obscure the difference. A useful performance claim has a baseline, workload, compiler, target architecture, compiled object and measured outcome.

For the smallest case, the separately compiled [[tools/m018/assembly_probe.cpp|assembly probe]] defines a templated generic sum of three floating-point numbers and an externally visible C-named wrapper. Build its assembly through the script or with:

~~~bash
g++ -std=c++20 -O2 -S tools/m018/assembly_probe.cpp -o /tmp/m018-gcc.s
clang++ -std=c++20 -O2 -S tools/m018/assembly_probe.cpp -o /tmp/m018-clang.s
~~~

Locate **allmight_m018_sum3** in each output. Ask whether the instantiated template was inlined, whether any call remains, whether the expected arithmetic appears in registers, and whether additional stack traffic or allocations are present. An x86-64 compiler may use scalar floating-point instructions, while an AArch64 compiler will have different mnemonics. It would be incorrect to demand one exact instruction sequence on every machine or infer a cycle count from the number of assembly lines alone. Floating-point associativity rules, inlining visibility and optimization flags also matter. The code must be compared to an equivalently compiled handwritten baseline to support a meaningful zero-cost claim.

For a real asset-catalog workload, count PIMPL allocations, name-copy allocations, lookups, cache misses and time per operation for several asset counts and lookup distributions. Compare an appropriately optimized flat vector, a sparse-set-like design and a hash lookup under equivalent semantics. Record cold versus warm cache, compiler version, flags, CPU type, allocator, thread count and variance. Profilers and disassemblers are instruments, not substitutes for a workload hypothesis. These requirements are explained **here**, rather than postponed to a future performance chapter; the next CPU phase can apply the same method to increasingly realistic machine behavior.

The provided lab's size assertions, generated assembly and standalone build checks are evidence for limited claims. It is not a broad benchmark, a stable cross-toolchain ABI demonstration or a proof that every language abstraction vanishes in machine code.

### The finished container contract is a set of explicit promises

The outward service is a reusable API only when its caller can answer the following questions without reading Impl: What object owns a resource? Can it be copied or moved? Which handles become invalid on erase, reuse, relocation or catalog destruction? Do query results borrow or own their data? Is failure atomic? Are lookups thread-safe? Does the catalog identify CPU metadata or actual GPU residency? What is allowed after the catalog moves?

For this teaching implementation, ownership is concentrated in AssetCatalog::Impl; catalog copying is forbidden and moving transfers its implementation identity. A handle is a value containing type, catalog identity, slot and generation. Erasing a valid resource destroys that slot's MeshInfo and advances its generation before reuse. A foreign, stale or out-of-range handle returns no result. Lookup returns an owned optional MeshInfo. Failed allocation during free-list preparation leaves the original record intact. The public class's private representation stays outside the header. Every callable on a moved-from catalog throws a documented logic error instead of dereferencing a null impl.

The lab checks these invariants using **check** functions that throw on failure, not the standard **assert** macro, because **-DNDEBUG** removes ordinary assert evaluations. That detail matters for the release-mode CI job. Additional compile-time assertions establish that mesh and texture handles are distinct, world and view vectors are distinct, meters and seconds are distinct, and built-in integers are excluded by this particular floating-scalar concept. The dynamic dot-product test verifies both a known result and mismatched-length rejection. The intrusive example checks duplicate insertion, LIFO order and unlinking. Sanitizers detect many memory errors on executed paths but are not exhaustive lifetime proofs.

There are still explicit limits. No concurrent catalog reader/writer execution is demonstrated. No GPU in-flight resource retirement is implemented. A GPU submission referencing a mesh buffer does **not** become safe merely because the catalog's CPU MeshInfo is generation-checked; the buffer must remain valid until the relevant GPU work has completed, and device destruction or reset introduces additional epochs. No allocator fault-injection matrix proves every possible exception transition. No serialized handle is guaranteed meaningful after restart. The sample does not implement a full dimensional algebra or an alias-safe expression-template library. Those are separate engineering requirements, not omissions to disguise as established features.

The practical result is one reconstructed design rather than ten detached terms. A selected mesh begins as a value with a stable logical identity; its live data is privately owned, its public service remains compilable independently, its mathematical operations carry stronger types, its temporary computations have stated lifetimes, and its performance claims require evidence. This closes the C++ design bridge to the CPU, data-structure and engine-implementation work without claiming that one small teaching lab is a production engine.

**Source and scope.** This is original authored reconstruction aligned with the exact M018/291–300 contract in the integrated curriculum, not a newly received original source transcript. Primary contracts checked for this chapter include [cppreference's incomplete-type/unique_ptr rules](https://en.cppreference.com/w/cpp/memory/unique_ptr), [the PIMPL explanation](https://en.cppreference.com/w/cpp/language/pimpl), and the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines). The supplied *Professional C++* and *Effective Modern C++* books supply broader object/lifetime and modern API design context; their older examples should not be misrepresented as post-C++20 ABI guarantees. Current KairoMath/Transform and KairoRenderer/RendererTypes were **not** audited by this reconstruction. The executed lab/CI and the source-to-part evidence are tracked separately below.

## From a correct C++ asset service to the instructions a CPU can execute — Phase 4, M019–M021, Parts 301–346

Our M018 catalog solved a language-level problem: an editor can carry a typed mesh handle across vector reallocations, deletions and slot reuse without accidentally treating a texture as a mesh. The catalog's ownership and invalidation rules are explicit. Nothing in that proof tells us how fast a render-world update will run. The CPU does not execute the words "handle," "range," or "PIMPL"; it fetches machine instructions, obtains data from a hierarchy of memories, tracks dependencies and eventually retires instructions. We now need to reconstruct the behavior of the same asset and scene-workflow at that level.

Imagine that the editor has loaded 4,096 particle-like scene records. Every frame, a simulation step needs only position and mass, but an editor-facing representation also stores debug flags, names, authoring data and auxiliary attributes. The update is mathematically trivial: sum each record's x-coordinate multiplied by its mass, then move active particles. Yet the result can be limited by instruction dependencies, data layout, cache traffic, virtual-memory translation, branch behavior or parallel coordination. The apparent simplicity of the mathematical function is why it is a useful experiment: if the cost is high, we must discover *which machine resource is actually constraining the work* rather than decorating the code with optimization vocabulary.

The original Phase 4 curriculum assigns **M019 = Parts 301–310**, **M020 = Parts 311–324** and **M021 = Parts 325–346** to this first CPU journey. The original titles remain preserved in the [[Supplementary/Foundations#Phase 4 CPU architecture and measured execution — Parts 301–400|phase coverage contract]]. The connected [[tools/labs/phase4_cpu_systems_lab.cpp|CPU systems laboratory]] provides representative, executable invariants; not every machine feature is testable by a portable C++ value assertion. Primary outside bridges are [MIT 6.172](https://ocw.mit.edu/courses/6-172-performance-engineering-of-software-systems-fall-2018/video_galleries/lecture-videos/), especially assembly, compiler limitations, caching and multicore lectures; [Agner Fog's optimization manuals](https://www.agner.org/optimize/), whose per-instruction tables are architecture-specific rather than ISO C++ rules; and the supplied *Game Engine Architecture* chapters on memory management and profiling. These are comparison sources, not claims that every video was watched or every book page was audited during this pass.

### Why a five-stage pipeline is an explanation, not a prediction of modern silicon

Start with the familiar teaching model: an instruction is fetched from instruction memory, decoded into an operation, executed by some functional unit, accesses data memory where necessary, and eventually commits or retires its architectural result. An elementary five-stage processor makes this sequence visible because each stage can be occupied by a different instruction. Suppose five independent instructions each require one stage per cycle. Without overlapping execution, they need roughly twenty-five stage-cycles; with an ideally filled pipeline, the first result still incurs pipeline startup latency, but later results may retire every cycle. This distinction between **latency** (delay from a particular input to its result) and **throughput** (amount of work completed per unit time) will recur throughout graphics computing. Pipeline depth is not automatically a speedup: longer pipelines increase latency and can amplify recovery after a wrong branch prediction.

Real out-of-order superscalar CPUs do not generally pass every instruction through exactly these five physical boxes. They may decode instructions into micro-operations, rename registers, issue several instructions per cycle to multiple execution units, execute out of order when their data operands are ready, and retire architectural effects in a constrained order. The five-stage drawing is a conceptual scaffold, not a substitute for a processor-specific execution model. If a professor asks why a CPU could complete a later arithmetic instruction before an earlier cache-missing load, the answer is not that source order has stopped mattering: the CPU can make speculative progress on independent operations while preserving the ISA's required architectural behavior and exception/retirement rules.

Trace a hot asset loop. One instruction loads the record's mass, another loads x, a third multiplies them and a fourth adds the product to an accumulator. The multiplication cannot begin before its required load values are available: this is a **read-after-write (RAW)** true dependency. But loading the next record's x or mass may be independent, enabling overlap. **Write-after-read (WAR)** and **write-after-write (WAW)** name other kinds of apparent register or storage ordering pressure; register renaming can remove *false name dependencies* when the ISA and microarchitecture permit it. Genuine memory aliasing is harder: the compiler and CPU cannot assume arbitrary stores and loads are unrelated without a justified alias and ordering analysis.

A simple recurrence makes this precise. For a scalar reduction

\[
s_{i+1}=s_i+x_i m_i,
\]

every new value of s depends on the previous sum. Even if loads and multiplications overlap, the accumulation itself forms a dependency chain. Multiple independent partial accumulators can shorten that chain, but changing the grouping changes floating-point rounding. A compiler using permitted vector or fast-math transformations can choose a different evaluation tree; strict language and compiler options limit which transformations are legal. Therefore a proposed faster reduction must state **both its numerical tolerance and its reassociation policy**.

Instruction-level parallelism (ILP) is the independence available among operations within one instruction stream. Superscalar issue width is a hardware upper bound on how many appropriate operations can be issued in a cycle, not a guarantee that every instruction sequence reaches that width. Execution-port conflicts, dependent chains, branch recovery, cache misses, front-end bandwidth and register pressure reduce the realized rate. A useful first model is

\[
T_{\text{CPU}}\approx \frac{I\,\mathrm{CPI}}{f},
\]

where I is the number of executed instructions, CPI the *measured average* cycles per instruction and f the frequency. CPI is an aggregate result of a workload and machine, not an intrinsic constant for one C++ expression. Instructions that retire several per cycle can produce CPI below one; stalls can make it much larger. Frequency scaling and heterogeneous core placement further invalidate naive cycle-to-wall-time assumptions.

Registers hold immediate operands and intermediates close to execution units. They are finite. If an optimization creates too many simultaneously live values, some may be **spilled** into stack memory, adding loads and stores. This is *register pressure*. Inlining an enormous templated function or unrolling a loop aggressively may increase pressure enough to hurt performance even when instruction count is reduced. We should inspect compiler assembly and profiling counters before pronouncing more unrolling better.

Now consider branches. The loop can skip retired assets by checking an active flag. A modern predictor uses history and patterns to guess the next instruction address before the condition is resolved. A correct prediction keeps the pipeline supplied with useful instructions; a misprediction discards work along the wrong control path and restarts at the correct path. A rough branch-cost contribution is **mispredictions × average recovery cost**, but the actual penalty depends on the processor and the surrounding code's ability to overlap other work. A branch that is almost always taken may cost far less than an unpredictable branch on scattered flags, yet replacing every branch with conditional masking is not always faster. Masking may evaluate operations for inactive elements and increase memory traffic or instructions.

Speculative execution is the hardware's attempt to execute predicted future work before all earlier control conditions have retired. Its transient effects are not equivalent to C++-observable committed results; security considerations show why speculation cannot be thought of as completely invisible at the microarchitectural level. Do not build a correctness argument on an assumed predictor state or on speculative instructions "never touching memory." Use the language memory model for program correctness and architecture-specific documentation for speculative-performance or side-channel reasoning.

Our first reconstruction task therefore has a concrete output: disassemble an optimized scalar traversal, identify which loads depend on which addresses, which arithmetic operations depend on previous results, which conditions can mispredict, and whether additional unrolling increased or reduced live-register pressure. It is legitimate for two optimized builds to use different instruction sequences; their differences must be explained with compilation flags, target ISA and program semantics. This is the exact role of Parts 301–310, not ten disconnected definitions.

### When memory traffic becomes more expensive than arithmetic

The asset loop reaches into main memory through multiple levels of storage. A core typically has small, comparatively fast L1 caches, larger/slower L2 caches and some form of shared or distributed last-level cache, backed by DRAM. Exact sizes, associativity, inclusivity and latency vary by CPU, platform and topology. A **cache hit** finds the requested data in the addressed level; a miss causes a request to another level. One source-level load may interact with line-fill buffers, prefetchers, store queues and other mechanisms, so counting source loads does not directly count DRAM transactions.

A cache does not normally fetch a single isolated four-byte float. It obtains a **cache line**, commonly 64 bytes on many current general-purpose CPUs but not universally. If we need one float from a large 64-byte record and never touch the rest, the line brings mostly irrelevant data. If we instead traverse a contiguous array of four-byte x values, each 64-byte line can contain sixteen useful floats, subject to alignment, line size and actual cache implementation. The useful fraction of transferred bytes therefore depends on how the algorithm walks storage, not on how many arithmetic operations appear in its source.

**Spatial locality** means nearby addresses are likely to be used close together; **temporal locality** means a recently used address will be accessed again soon enough that retaining its line is useful. A sequential scan of densely packed elements often benefits from spatial locality and hardware prefetching. Pointer chasing through an unpredictable linked structure can create a dependency chain in which the next address is unknown until the current node is loaded. Even if a linked list has the same O(n) traversal complexity as a vector, it can have radically different stalls due to line utilization, prefetchability and allocation locality. Big-O does not model those constant factors or data distribution.

Caches are not fully associative infinite dictionaries. A conventional set-associative cache maps addresses to sets, and each set has a limited number of ways. After the line offset bits, some address bits participate in set selection; remaining address information forms the tag (with actual indexing and physical-address constraints depending on the cache). If many simultaneously needed lines map to the same set, they may repeatedly evict each other despite free capacity in other sets. This is a **conflict miss**; an initially unseen line can produce a compulsory miss; a working set too large for effective cache capacity can produce capacity misses. We should test layouts that produce set conflicts rather than assume every performance drop is an L3 miss.

An elementary cache address exercise makes the logic reconstructible. Suppose a hypothetical 32 KiB, eight-way, 64-byte-line cache. It holds 32,768/64 = 512 lines, distributed across 512/8 = 64 sets. A byte-addressed line has six offset bits, and this simple model needs six bits to choose one of 64 sets. Sequential addresses separated by 64 bytes step through lines, while addresses separated by 64 × 64 = 4,096 bytes can map to the same set in this simplified indexing model. Nine distinct such lines competing for eight ways can cause repeated eviction if all are needed and mapping/physical-tag assumptions hold. Real processors can use more complicated hashing, virtual indexes and policies, so the arithmetic is an architectural reasoning exercise, not a claim about an Apple or Intel cache's exact mapping.

Prefetchers may recognize predictable sequential or strided streams and begin loading future cache lines before an explicit load demands them. A pointer graph with random next links is less predictable. Prefetching can help hide latency but also consumes bandwidth and cache capacity; an ineffective prefetch may evict useful data. It is helpful to separate **latency**—the time for an access dependency to complete—from **bandwidth**—the sustained volume transferred per second. Many independent outstanding loads can saturate bandwidth despite high individual latency. A serialized pointer chase may be latency-bound despite using little total bandwidth.

We can estimate a bandwidth lower bound when the memory-bound conditions really apply:

\[
T \;\geq\; \frac{\text{bytes that must cross the limiting interface}}
                    {\text{sustainable bytes per second}}.
\]

For a purely illustrative machine capable of sustaining 20 GB/s on the relevant transfer path, moving 2 GB of indispensable data takes at least 0.1 s on that path. The estimate is only useful if the bytes and the interface are correctly identified. Cache reuse can reduce DRAM traffic; write allocation, coherence, other threads and prefetching can increase it. We should never take the structure's size and automatically equate it to memory-bus traffic.

Virtual addresses introduce another layer. The program operates with virtual addresses that hardware and the OS translate to physical memory using page-table structures, with a **Translation Lookaside Buffer (TLB)** caching recent translations. A TLB miss may require walking page tables, sometimes generating extra memory accesses. Traversing many widely separated pages can exert TLB pressure even if each access touches only a few bytes. Larger pages can reduce the number of translations needed for the same contiguous region, but huge-page availability, fragmentation, OS policy, permissions and portability are real constraints. Huge pages are not a universal "speed" flag. The lab does not allocate huge pages, and no improvement is claimed from merely mentioning them.

On multiprocessor systems with non-uniform memory access (NUMA), the cost of DRAM may depend on the physical node holding a page and the node where the executing core is scheduled. First-touch placement, thread pinning, affinity, allocator behavior and cross-node interconnect traffic become relevant. Machines with a unified memory architecture or heterogeneous CPU clusters still have topology-dependent performance characteristics, but the appropriate model must be learned from their own memory system rather than imposing a conventional multisocket NUMA diagram. The current Phase 4 laboratory does not enumerate or pin NUMA nodes.

The difference between a cache-line miss and a TLB miss is especially important when debugging: one concerns the location of data within the memory hierarchy; the other concerns translating the address. A source-level "slow load" can have both costs, neither, or be stalled on another dependency. Use performance counters and controlled experiments to tell them apart.

### Rewrite the representation around access patterns, not names

Return to the 4,096 records. A conventional **array of structures (AoS)** stores one complete Particle object after another. It is convenient for editing one object at a time. Our demonstrative AoS contains four hot floats (x, y, z, mass) and twelve cold floats used by editor/debug features: sixty-four bytes of float fields in total before any additional layout effects. A hot pass reading only x and mass may transfer more unused data than necessary if it scans large cold-rich objects. A **structure of arrays (SoA)** instead keeps all x values contiguous, all mass values contiguous, and so on. Two hot streams can be loaded efficiently, and the compiler may more easily vectorize them.

An **array of structures of arrays (AoSoA)** divides records into small blocks, with arrays of each field inside each block. This can combine block-local entity operations with SIMD-friendly lanes. The right block width depends on target vector widths, cache behavior, alignment and the algorithm's actual accesses; it is not universally 4, 8 or 16. Hot/cold splitting is another independent choice: store rarely used metadata elsewhere so a per-frame update does not repeatedly pull it through the cache.

Compare the *logical* bytes touched by an illustrative x-and-mass operation. Each ParticleAoS in our lab has 64 bytes of float data; an SoA pass may traverse only two four-byte arrays, or 8 useful bytes per entity. That suggests an eightfold difference in sequential stream volume **in this declared representation**, before cache reuse, line fills, compiler loads, write traffic and alignment. The actual speedup need not be eightfold. If the working set already fits in L1, or the math dominates, or the compiler's vectorization differs, or the AoS is reused for many other attributes in the same pass, the results change. This is a hypothesis to benchmark, not a numeric benchmark result.

The [[tools/labs/phase4_cpu_systems_lab.cpp|Phase 4 CPU lab]] constructs the same exactly representable values in both layouts, sums their x × mass values, and verifies equality against 30,720. It also compacts active indices into a dense vector. **Compaction** converts a sparse set of survivors into a contiguous traversal-friendly representation, often using filter/scan/scatter on a GPU or a serial conditional append on a CPU. It has an upfront cost, so it is justified when the saved downstream work outweighs that cost. Existing stable logical handles must not be confused with new dense physical positions: a separate index map or generation checked reference maintains identity.

~~~cpp
float sum_aos = 0.0f;
for (const ParticleAoS& p : aos)
    sum_aos += p.x * p.mass;

float sum_soa = 0.0f;
for (std::size_t i = 0; i < count; ++i)
    sum_soa += soa.x[i] * soa.mass[i];

// In the lab, both arrays hold deliberately exactly representable values.
// This tests equivalence on the chosen data, not universal bitwise equality
// for arbitrary floating-point inputs or differing reduction trees.
~~~

An ECS can use archetype chunks or sparse sets for related reasons. But the same "put data in contiguous arrays" slogan cannot settle an engine's overall design. Component addition and removal, stable entity identity, hierarchy, event delivery, editor metadata, script references and render snapshots all contribute. We must optimize the operations actually repeated in the hot path while stating which operations become more expensive. The performance result is workload-relative.

### Sharing a cache is not the same as safely sharing a value

Now parallelize scene updates. Suppose two threads increment independent counters in adjacent locations. If both counters occupy a single cache line, each core's writes can force coherence activity even though the values are distinct and the C++ program has no data race. This is **false sharing**. A data race is a language-level failure when conflicting non-atomic accesses lack required happens-before ordering; false sharing is a *performance* cost from coherence operating at a granularity larger than the variables.

A cache-coherence protocol such as a MESI-style model tracks line states that can be summarized as Modified, Exclusive, Shared and Invalid. Actual protocols include many additional states and variations. A core cannot simply write any shared copy while other cores simultaneously assume it is unchanged; the protocol arranges ownership, invalidation or other legal transitions. A pair of adjacent counters repeatedly written by different cores may cause the line to migrate. Conversely, two counters in separately aligned regions can reduce that particular interference, but padding increases memory footprint and may harm other access patterns.

The lab models a common mitigation:

~~~cpp
struct alignas(64) PaddedCounter {
    std::atomic<std::uint64_t> value{0};
};

// Two threads increment different counters using relaxed atomic operations.
// 64-byte alignment is a declared hypothesis, not a guarantee that every
// target machine has 64-byte destructive interference granularity.
~~~

The test verifies the final counts. It does **not** prove improved throughput, because it does not compare padded and unpadded variants under a controlled noisy multicore workload. It also does not prove that alignas(64) always separates every target's actual cache-coherence units. The important reconstruction is *why* a line-based ownership protocol can impose an extra cost even when distinct variables are individually race-free.

Hardware memory fences and the C++ memory model must not be conflated. A CPU may reorder or buffer accesses internally subject to its ISA rules; a compiler may also reorder operations subject to C++ observable-behavior rules. A source-level std::atomic operation with release or acquire semantics constrains which operations can be published and observed between threads when the synchronization conditions actually hold. The final CPU instructions implementing those semantics vary by architecture. A fence instruction is neither a magic cache flush nor a universal cure for incorrect ownership and lifetime.

### SIMD: one instruction, several elements, but only under a valid contract

A Single Instruction Multiple Data (SIMD) operation performs the same kind of calculation on multiple lanes of a vector register. A simple vector-addition instruction might add four floats or eight floats at once on one ISA, while another ISA uses different vector lengths or instructions. SIMD improves *potential* arithmetic throughput and can reduce instruction overhead, but it still needs data, registers, issue bandwidth and stores. It does not guarantee faster wall-clock time when memory traffic dominates.

For a loop processing N elements in packets of W lanes, the main body executes roughly floor(N/W) packets, with a **tail** of N mod W elements. A portable implementation can process the tail with a scalar loop; an architecture-specific implementation may use masked loads/stores. Reading or writing beyond the valid buffer merely because the register contains unused lanes is not allowed. Masked instructions also require checking what the ISA promises about inactive-lane address validity and exceptions. A safe tail strategy is part of correctness, not a tuning detail.

Alignment means that an address is a multiple of a required boundary. Some SIMD loads benefit from or require alignment; others accept unaligned addresses with possible architectural or workload-dependent costs. ISO C++ object alignment, allocator guarantees and an ISA's memory-operation requirements are related but distinct. A programmer cannot assume a vector's data pointer is aligned to a particular extended SIMD width merely because the element is a float. If using aligned instructions, establish or check alignment through the correct allocator and type contracts.

Horizontal reduction combines multiple lanes into one result. It can create a tree of additions rather than a left-to-right scalar chain. For floating-point input, that tree may round differently. Likewise, a branchless SIMD mask can avoid branch divergence while still doing unused operations; gather and scatter can access noncontiguous addresses but often lose the advantage of contiguous streams. Gather/scatter should be chosen from actual address patterns, not because the instructions sound more advanced.

Auto-vectorization depends on the compiler proving enough about loop independence, aliasing, trip counts, alignment and target instruction support. Conditional code, indirect indexing, potential exceptions and unclear alias relationships may block profitable transformations. The practical evidence chain is: compile with explicit optimization and target options, request the compiler's vectorization report, inspect the relevant machine code, check numerical correctness, then benchmark a realistic workload. The absence of a vectorization message does not alone prove poor performance; the presence of one does not prove speedup.

The portable source in the Phase 4 lab leaves SIMD selection to the compiler and does not use architecture-specific intrinsics. This is intentional: it exercises *what can be checked portably*. A later x86 AVX or ARM NEON specialization must declare feature detection, instruction availability, alignment, tails, numerical rounding, fallback behavior, benchmark methodology and CI hardware constraints. Before doing that, compare it to the compiler's optimized scalar/SoA implementation. The result may show that the compiler already produces a better instruction sequence.

The compiler's fast-math options can allow transformations that violate strict IEEE-style assumptions including signed zero, NaN/infinity behavior or reassociation, depending on the precise options. Such flags are a semantic change, not a universally safe rendering optimization. For physics constraints, geometric predicates, randomized reproducibility and research experiments, numerical assumptions may outweigh a small throughput advantage. Do not apply permissive floating-point flags globally without testing the algorithms that depend on stable numerical behavior.

### One measurement chain for the first half of Phase 4

We now have an inspectable causal chain: C++ source defines a sequence of hot accesses; the compiler emits instructions constrained by aliasing and floating-point semantics; the CPU fetches and schedules those instructions; cache and TLB behavior determine data availability; layout determines how many bytes are useful; and parallel writing can trigger line-level coherence costs. Each layer supports a different kind of evidence. A passing source test can establish that the AoS and SoA loops compute equal selected results. Compiler reports and disassembly can establish an observed vectorization strategy for one target. Hardware counters and careful timing can support claims about stalls or bandwidth. None automatically proves the others.

To make the experiment repeatable, record exact input count, data generation, compiler/version, optimization and target flags, CPU model, thread count, warm-up, repetitions and summary statistic. The optional benchmark in the lab reports median timings for seven executions of each layout but intentionally has **no pass/fail speed threshold**. The result must be replicated on target hardware and interpreted as a comparison for one access pattern. If the speedup changes after accessing more fields, that is useful evidence about workload dependence rather than an embarrassment to hide.

The lab and this narrative complete the *authored explanation* for M019–M021, Parts 301–346. They do not claim to have independently profiled KairoECS/SparseSet, KairoSIMD or Maveb's actual JobSystem. Those repository anchors remain to be inspected with exact source/function revisions before any statements about their current layout or optimizations.

## From temporary memory to finished parallel frames — Phase 4, M022–M024, Parts 347–400

Our CPU-side scene pass is now intelligible at the instruction, cache and data-layout levels. But an engine does not only update values that already exist. It allocates temporary arrays, loads and retires mesh data, creates or schedules worker tasks, publishes results to readers, waits for dependencies and eventually submits work to a GPU that can still be reading resources after the CPU returns. The performance and correctness problems now form one connected question: **who owns each byte, which execution may access it, how is that access ordered, and what is actually limiting the frame?**

This is the natural home for M022 (Parts 347–360: allocator strategies and deferred destruction), M023 (361–384: threads, atomics, memory ordering and synchronization) and M024 (385–400: job systems, measured parallelism and profiler-based decisions). The code witness is the same [[tools/labs/phase4_cpu_systems_lab.cpp|CPU systems laboratory]], where frame arenas, a counted allocator, an owned retirement queue, acquire/release message publication and an actual persistent worker pool are checked. Work stealing and GPU-completion integration are explained as distinct, still-unimplemented algorithms rather than implied by a generic thread-pool test.

### Each allocation makes two promises: suitable storage and controlled lifetime

The general-purpose heap allocator answers requests for dynamically sized storage with a suitable alignment and a lifetime that can extend past the current function. Its goals can include efficient small allocations, acceptable fragmentation, low tail latency, safe multithreaded use and bounded metadata overhead. These goals conflict: a general allocator that handles arbitrary sizes and lifetimes cannot assume every object disappears at the end of a frame.

An allocation does **not** construct every desired C++ object by itself. Storage acquisition reserves appropriately sized and aligned bytes; object construction starts a typed lifetime in valid storage. Using **std::construct_at** or placement new on suitably aligned raw storage is a distinct operation from obtaining the bytes. Destruction ends the object's lifetime and may need to run its destructor before storage is returned or reused. This matters for resources holding strings, file descriptors, mutexes or GPU handles: simply resetting a bump pointer does not automatically call all their destructors.

Different ownership patterns suggest different allocation strategies. A free-list allocator maintains reusable blocks and can coalesce adjacent free blocks when its representation and policy support it. A stack allocator can roll back only in last-in-first-out order, often with markers rather than individual frees. A linear or arena allocator advances a cursor and releases storage in bulk, making allocation cheap when all objects share a lifetime boundary. A pool allocator serves fixed-size objects from slots; a slab allocator commonly groups objects of one size class into larger backing slabs. A buddy allocator partitions power-of-two blocks and can merge matched buddies when both become free. A size-class allocator chooses bins for ranges of request sizes and trades memory waste for speed and predictable reuse. None is best independent of the size/lifetime distribution.

**Internal fragmentation** is allocated space not used by the requested object, such as padding or rounding a 100-byte request up to a 128-byte size class. **External fragmentation** describes free storage split into blocks that cannot satisfy a needed larger contiguous allocation despite sufficient total free bytes. A simple aggregate percentage of wasted bytes cannot identify all causes of allocation latency. Metadata cost, thread-local caches, lock contention, page retention, NUMA placement and over-alignment should also be measured. A pool can reduce allocation cost for repeated small objects but may waste memory when its capacity remains unused or when object lifetimes vary widely.

Alignment constrains the addresses at which objects may reside. If a type needs alignment a, its address must be divisible by a under the relevant C++/ABI definition. Over-aligned allocations may require an allocation mechanism that honors the greater alignment. For a linear arena with current cursor p and power-of-two a, a familiar alignment expression is

\[
p_{\mathrm{aligned}}=(p+a-1)\mathbin{\&}\sim(a-1),
\]

but a correct implementation must ensure that p+a−1 does not overflow, that a is valid, that the aligned extent fits the backing allocation, and that the returned pointer meets the type's actual alignment. A robust arena should reason about byte spans and capacity *before* pointer arithmetic. The formulas are not license to write past a buffer.

A **frame arena** is appropriate for scratch arrays that unquestionably die together after a well-defined frame or job epoch. Our lab gives a std::pmr::monotonic_buffer_resource a small aligned local byte array and null upstream. A std::pmr::vector stores one hundred integers in that region; an intentionally unreasonable reserve request exhausts it and throws std::bad_alloc rather than invisibly allocating a second backing region. This makes the capacity boundary observable. Destruction of the vector runs its element destructors, but a monotonic resource normally retains its backing blocks until it is itself released or destroyed. Treating a monotonic buffer like an ordinary per-object free list is conceptually wrong.

The same test uses a counting std::pmr::memory_resource upstream of an ordinary PMR vector. It counts successful acquisitions and matching deallocations in a controlled scope; that proves those calls occurred for this tested vector. It does not prove zero heap fragmentation or allocator scalability under concurrency. The arena and its users have a lifetime dependency: a container or string allocated from that resource must not outlive the resource, even if the container itself is a valid C++ object.

These ideas also explain why a per-frame allocator can make an engine *incorrect* if used carelessly. The CPU's notion of "frame complete" is not the GPU's. A command buffer might reference a buffer and return control to the CPU before the GPU has executed it. If a frame arena backs data that a GPU copy has not yet consumed, resetting the arena can overwrite in-flight bytes. Double- or triple-buffering of transient resources may help but does not alone prove safety: completion tokens, queue dependencies, device-loss handling and reuse of each specific allocation must be correct.

### Deferred destruction is ownership plus a completion relation

M018 defined logical asset lifetime using a generation-checked handle. A mesh can become invalid to new CPU lookups immediately after deletion. Yet a previously submitted GPU draw may still legally reference the underlying GPU buffer. It is therefore necessary to distinguish **logical retirement** from **physical destruction**.

The owner marks a resource logically unavailable, records the last GPU submission that can use it, and defers actual destruction until a trustworthy completion signal proves that submission—and any relevant dependent uses—has finished. A monotonic completion timeline is a useful model, provided all relevant uses belong to or are correctly joined into that timeline. The stored resource owner remains alive in the retirement queue.

Our CPU simulation uses integer tickets. We enqueue resources at tickets 2, 5 and 5. After reporting completion of ticket 2, exactly one owned object may be released. After ticket 5, the other two can be destroyed. A completion value going backwards is rejected. This proves the queue's *integer-order and ownership mechanics* on those test cases, not that an actual Vulkan semaphore or Metal command buffer was observed.

A real graphics implementation needs actual API synchronization. For multiple GPU queues, a single scalar ticket may be insufficient unless submission dependencies explicitly establish a common ordering. Queue-family ownership transfers, device-loss paths, wait failures, timeline wrap, external resources and non-GPU users need their own contracts. Vulkan's host-visible signaling and resource lifetime must be interpreted under the current specification; Metal completion callbacks and command buffer status similarly need accurate ownership and callback-lifetime rules. Releasing GPU handles just because the CPU emitted commands is a use-after-free hazard even when every C++ pointer in the catalog is valid.

One subtle failure mode is the opposite: a retirement queue can retain too many resources and exhaust memory if the oldest GPU work never completes or the application stops processing completion events. It needs diagnostics for queue depth, oldest pending ticket, bytes retained, device progress and shutdown policy. A correct engine cannot silently free live resources during a timeout; it must choose a defined error and teardown path.

### Threads share a process address space, not a common execution order

The editor wants to prepare visibility and mesh reports while the main thread continues handling input. A std::thread starts independent execution within the process address space; objects can therefore be accessed by multiple threads. Creating and destroying threads is not free: it can involve kernel scheduling resources, stack setup, runtime initialization and later joining. A context switch can change the running thread and incur scheduling and cache effects. The actual cost depends on the OS and workload, but repeatedly creating one thread for every tiny mesh task is normally an unexamined overhead risk. Persistent worker pools amortize it.

Sharing memory is not the same as safely sharing memory. In the C++ memory model, conflicting accesses to the same non-atomic location by different threads without the required happens-before relation can cause a **data race**, whose behavior is undefined. A counter that sometimes "looks correct" in a release test is not proof of a race-free program. A volatile qualifier is not a C++ thread-synchronization primitive.

Atomic operations provide indivisible operations on their own objects, but their **memory-order semantics** determine how they relate to other memory accesses. A relaxed fetch_add on an atomic statistic can count events without establishing ordering for unrelated payload data. It does not safely publish the contents of a separate ordinary object to another thread by itself. Release and acquire are used when one thread must make preceding writes visible to another that observes the publication.

In the Phase 4 lab, the producer writes an ordinary integer payload and then stores ready=true with **memory_order_release**. The consumer repeatedly loads ready with **memory_order_acquire**. When the acquire load observes the value written by the release store, the release synchronizes with that acquire, establishing the required happens-before relation for the earlier payload write. The consumer may then safely read the payload, provided no other unsynchronized thread concurrently mutates it. Merely having acquire on one atomic and release on an unrelated atomic is not enough.

~~~cpp
// Producer: ordinary payload write is sequenced before release publication.
message.payload = 42;
message.ready.store(true, std::memory_order_release);

// Consumer: only after it observes the published value may it read payload.
while (!message.ready.load(std::memory_order_acquire))
    std::this_thread::yield();
const int published_value = message.payload;
~~~

**Acquire-release** on a read-modify-write operation combines acquiring information from preceding releases with publishing this operation's earlier writes under the relevant conditions. **Sequential consistency** adds a strong single-total-order model for sequentially consistent atomic operations, but it does not automatically protect arbitrarily accessed non-atomic objects or eliminate all ordering questions. The weakest correct ordering must be derived from a specific protocol and verified on the target architectures; starting with simple locks or stronger atomics is often preferable to premature relaxed-atomic cleverness.

**Compare-and-exchange (CAS)** attempts to replace an atomic value only if it equals an expected value. It is indispensable in many lock-free algorithms but not sufficient for safe memory reclamation. An **ABA** problem occurs when a location changes from value A to B and back to A between observations; a CAS testing only the current bits may incorrectly conclude that nothing relevant changed. Generational identities, tagged pointers or other versioning can distinguish some ABA cases, but the lifetime of objects behind atomic pointers still needs safe reclamation such as hazard pointers, epochs, reference counting or a carefully designed ownership protocol. A generation-checked MeshHandle does not make a lock-free queue safe by itself.

Locks remain useful. A mutex protects an invariant by making critical sections mutually exclusive. Its unlock/lock synchronization also orders accesses to protected state. A reader-writer lock may allow concurrent readers when contention patterns justify it, but writers can starve under some policies; reader-heavy is not synonymous with faster. A spinlock repeatedly tries ownership and can waste CPU under preemption or long critical sections; it is generally unsuitable when sleeping, blocking I/O or expensive work happens inside the protected region. Lock choice must follow measured wait times, critical-section duration, fairness and scheduler behavior.

A condition variable solves a different problem: a worker needs to sleep until work appears. The queue state remains protected by a mutex, and the wait must use a **predicate**. Spurious wakeups can occur, and a notification does not store an event forever like a queue message. A worker that wakes must recheck the shared condition while holding the lock. The laboratory's worker checks whether stopping is requested or tasks are nonempty before taking a job. It executes the job *outside* the queue lock so other producers and workers can make progress.

~~~cpp
std::unique_lock lock(mutex_);
changed_.wait(lock, [&] {
    return stopping_ || !tasks_.empty();
});
if (stopping_ && tasks_.empty())
    return; // Shutdown only after queued work has drained.
auto job = std::move(tasks_.front());
tasks_.pop_front();
lock.unlock();
job(); // User work never runs while the queue mutex is held.
~~~

Counting semaphores represent available permits and can coordinate bounded queues or resources without the exact same predicate structure as a condition variable. Latches wait for a count of arrivals; barriers coordinate repeated phases and may have completion actions. They are not interchangeable: a worker waiting for queue work has different semantics from a set of threads that must all finish one simulation phase before any begin the next.

**Deadlock** occurs when a set of waits cannot progress, often through circular acquisition of resources. A lock ordering discipline eliminates a class of cycles: if code always acquires mutex A before mutex B, it cannot also acquire B and then wait for A through a reversed order. std::scoped_lock can acquire multiple mutexes using a deadlock-avoiding mechanism. But blocking on a future while holding a lock needed by the future's producer can still deadlock; so can tasks waiting on tasks from a fixed-size pool with no free worker. **Livelock** is continued activity without useful progress; **starvation** is failure of a participant to receive needed service despite system progress. A lock can be race-free and still cause unacceptable tail latency or starvation under contention.

### A job system is a scheduling graph, not merely a vector of threads

A persistent **thread pool** keeps a bounded set of workers alive and reuses them across tasks. A **job system** must also answer how tasks are represented, how work becomes ready, how dependencies are enforced, how failures propagate, how shutdown happens and how work is distributed across workers. Different engines choose different solutions depending on task granularity, frame determinism and runtime targets.

The Phase 4 lab constructs a real small worker pool with a shared mutex-protected deque, a condition variable and packaged_task-backed futures. Callers submit jobs; each worker removes a job under the lock and executes it outside the lock. Exceptions thrown by a user job are stored in the packaged task and can be rethrown by future.get() in the caller. Shutdown changes a protected stop flag, wakes the workers, permits queued work to drain and joins all worker threads. These choices are correct for the exercised protocol; they are **not** work stealing or a complete dependency-aware production scheduler.

The test first partitions a 10,000-element array into fixed chunks, submits independent reductions and combines the returned subtotals in a fixed order. It uses values exactly representable as double, so the oracle is an exact integer-valued total of 15,000. It tests grain sizes 1, 127 and 4,096. That demonstrates correctness over different task partitions. It does not demonstrate that creating 10,000 tiny tasks is efficient. The cost of packaging, queueing, waking and collecting those tasks may dominate the few arithmetic operations each job performs.

A practical scheduler chooses **grain size** so each task contains enough useful work to amortize scheduling overhead but not so much that one worker remains busy after others finish. If a task costs c units of useful work and incurs overhead h, then for n tasks

\[
T_{\mathrm{overhead}}\approx n h,
\]

before accounting for contention, cache effects and critical path dependencies. Smaller tasks improve load balancing in irregular workloads but increase overhead. Coarser tasks reduce overhead but may create idle workers and poor balance. There is no universal ideal grain size expressed solely in bytes or number of particles.

**Work stealing** addresses load imbalance by letting idle workers take work from other workers' queues, commonly using per-worker deques with local and stolen ends. The owner and thief must obey carefully designed synchronization and reclamation rules. A work-stealing deque is significantly more difficult than one mutex-protected queue: empty/single-element races, index wraparound, false sharing, memory ordering and capacity growth all become correctness concerns. The current lab deliberately does not claim to implement it. MIT 6.172's parallelism and Cilk runtime material provides a path to deeper implementations and work/span analysis.

A **task dependency graph** makes execution constraints explicit. If animation must complete before skinning, and skinning before a visibility-dependent render preparation, scheduling unrelated work earlier is legal but crossing those edges is not. A directed acyclic graph permits topological readiness processing; a cycle in required dependencies represents a scheduling impossibility absent a separately designed feedback or iterative protocol. Dependencies should be represented as data and tested, not hidden in accidental submission order.

For a workload with total work W and critical-path length S, the work/span model gives a conceptual lower bound

\[
T_P\;\geq\;\max\!\left(\frac{W}{P},\, S\right)
\]

for P equal-speed processors under the model. W/P expresses total work capacity; S expresses serial dependency limitations. Real systems add scheduler cost, synchronization, memory effects and heterogeneous execution. A job system with a hundred workers cannot beat a single long dependency chain.

### Determinism is a property of the complete update contract

Parallel reductions expose an immediate determinism problem. Floating-point addition is generally not associative:

\[
(a+b)+c\neq a+(b+c)
\]

for some representable a, b, c under finite-precision rounding. If task completion order determines reduction order, two valid executions may obtain slightly different results even without a data race. For deterministic physics, reproducible experiments and binary comparison tests, the update must define its accumulation and iteration ordering (or tolerate and quantify numerical differences).

One strategy partitions work into fixed chunks independent of worker scheduling and combines each chunk's result in a fixed order. The Phase 4 lab demonstrates that approach on exact integer-valued doubles. However, changing the chunk size still changes the addition tree. Fixed partitioning by itself cannot promise bit-identical floating-point results across different CPU ISAs, compilers, FMA decisions or math flags. A stronger guarantee requires fixed arithmetic semantics, stable ordering and carefully bounded execution-environment differences; even then transcendental library implementations may differ.

Determinism also concerns *state changes*. If two tasks write overlapping transforms or update the same collision manifold, a fixed final reduction does not repair a data race. Use exclusive component ownership, explicit barriers, deferred command buffers with deterministic merge order, or partitioning rules that make write sets disjoint. The solver phase later applies these same principles to contact islands and constraint batching.

### Profile first: one plausible optimization can hide the real bottleneck

A performance discussion should end by making a falsifiable decision, not by naming CPU features. Suppose an engine frame takes 16 ms, and a profile suggests 6 ms of scene update, 4 ms of physics, 3 ms of rendering submission and 3 ms of other work. Even this breakdown is only a starting measurement: did we include GPU wait time? Does the profiler report inclusive or exclusive costs? Are the categories concurrent or truly additive? Did instrumentation change scheduling? Are these median frames or tail outliers? An apparently slow CPU function can be blocked on another resource, and optimizing its arithmetic may not shorten the frame's critical path.

A **sampling profiler** periodically observes execution stacks with relatively low overhead, yielding statistical evidence of where CPU time is spent. An **instrumentation profiler** records explicitly marked events or functions and can produce precise phase durations and counts but alters the workload through instrumentation cost. **Flame graphs** aggregate stack samples: their horizontal extent represents sampled frequency/weight under the collection method, not source execution chronology or necessarily actual wall-clock span. The tool should be chosen according to the question: instruction stalls need hardware-aware profiling; lock contention needs wait analysis; transient GPU latency needs CPU/GPU timeline correlation; memory ownership needs allocation traces.

A proper microbenchmark includes warm-up and repeated runs, measures enough work to exceed timer quantization, checks that computations were not optimized away, and reports distributions rather than one favorable number. Cache warmth, CPU frequency, thermal state, background processes, address-space layout and data alignment can all influence results. Statistical noise should be quantified through repeated independent runs, medians and appropriate uncertainty estimates. Comparing two implementations only after a code cleanup, different compiler option or different input is confounded.

A useful speedup model is **Amdahl's law**. If fraction s of the baseline execution is inherently serial and the remainder is accelerated by a factor P, then the idealized overall speedup is

\[
S(P)=\frac{1}{s+(1-s)/P}.
\]

With s=0.2 and P=4, the ideal speedup is 1/(0.2+0.8/4)=2.5, *not* four. As P grows without limit, the maximum under this fixed-fraction model approaches 1/s=5. The model assumes the work fractions and costs are meaningful and ignores many real overheads; it does not predict actual hardware utilization. It teaches us why profiling the serial fraction and the critical path matters before growing the pool.

A well-grounded optimization report should state: the original frame/workload and measurement setup; a hypothesis tied to one observed bottleneck; a code change that preserves required semantics; the new profile and end-to-end measurement under the same conditions; variance and failure cases; and whether the change improved the actual user-visible workload. If the profiler indicated expensive cache misses in an AoS pass, a SoA conversion should be evaluated in the complete engine with its cost for insertion, deletion, serialization and editor interaction. If the profiler showed job-queue contention, simply increasing worker count may make it worse. If GPU completion is the limiter, CPU arithmetic changes may not affect frame time at all.

**This closes the Phase 4 authored narrative for Parts 301–400** through actual causal dependencies: C++ objects become machine instructions; hot data induces cache and TLB behavior; allocator choice controls memory lifetime; ownership crosses CPU/GPU completion; threads require a memory-model proof; jobs require dependency and scheduling contracts; and any optimization must be justified by measurement. The accompanying laboratory tests only those portable properties it actually executes. It does not turn a toy thread pool into Kairo's real scheduler, claim GPU retirement tests, or prove speedups on hardware it did not profile.

**Repository/research bridge:** the canonical code anchors include KairoECS/SparseSet, KairoSIMD, KairoScheduler and Maveb's JobSystem/Profiler, but this vault-only development did not inspect their current source revisions or run their actual workloads. *Game Engine Architecture*, Part II, Chapter 5 motivates allocation policies, while its profiling chapter distinguishes statistical and instrumentation techniques; exact implementation examples in the 2009 edition remain historical. [MIT 6.172](https://ocw.mit.edu/courses/6-172-performance-engineering-of-software-systems-fall-2018/video_galleries/lecture-videos/) supplies primary teaching links for C to assembly, multicore, caching, storage allocation, profiling and graph optimizations. [cppreference's memory ordering](https://en.cppreference.com/w/cpp/atomic/memory_order) supports the release/acquire distinction. [Amdahl's 1967 paper](https://doi.org/10.1145/1465482.1465560) is the historical research source for the fixed-serial-fraction argument. None of these sources establishes that the user's existing engine has a particular bottleneck without direct evidence.

## The scene is now too large for the wrong container — Phase 5, M025–M027, Parts 401–445

The Phase 4 scene update taught us what it means for a data structure to be fast *on a particular machine*: a cache line may carry unused fields, a pointer chain can serialize loads, a branch can be unpredictable, and a parallel task may spend more time waiting than doing useful work. We can now make a more demanding engine-design decision. The editor is adding and deleting assets while a navigation system searches paths, a renderer groups spatial objects, a scheduler orders dependencies and a streaming system tracks the most recently used resources. None of them merely needs "a data structure." Each needs a precise *abstract operation* with a correctness contract, and each operation has a workload-dependent cost.

The natural unit of teaching is the engine workload, not the catalog of names. We will follow several representations of the same set of logical assets and paths, derive why their operations cost what they cost, implement selected representatives and establish which alternatives are still conceptual. The original M025 (Parts 401–410) owns complexity and amortization; M026 (411–430) owns contiguous, linked, queued and hashed storage; M027 (431–445) owns ordered trees, heaps, tries and disjoint-set union. Exact titles are preserved in [[Supplementary/Foundations#Phase 5 algorithms and data-structure workload contracts — Parts 401–500|the 100-part contract]]. The [[tools/labs/phase5_algorithm_workload_lab.cpp|algorithm laboratory]] implements a queue, a real open-addressing table with tombstones and rehashing, a Bloom filter and disjoint-set union, alongside downstream graph and scan oracles. It does **not** claim full AVL, red-black, B+ or Robin Hood implementations merely because those ideas are explained.

### What an abstract data type promises, and what a representation actually pays

A *set* promises operations such as insert, contains and erase. It does not promise that those operations use a sorted array, hash table, tree or bitmap. A *map* associates keys with values; a *queue* preserves first-in-first-out order; a *priority queue* extracts an element of highest defined priority rather than oldest arrival. These are **abstract data types (ADTs)**, specified by what results and state transitions are observable. A **data structure** is a representation that implements those operations, with a choice of memory layout and algorithms.

This distinction is practical for the M018 asset service. Its external handle validation could use a flat vector indexed by a slot, achieving expected constant-time indexed lookup with a few direct accesses, whereas mapping arbitrary asset names to handles may need hashing or an ordered tree. Replacing the name index with a B-tree should not change the public meaning of "find name"; it changes cost, iteration order and sometimes memory stability. An API that accidentally exposes iterators or raw pointers into one representation can make that replacement a breaking change. We therefore define the semantic contract before selecting a representation.

Let n denote the number of stored assets. An operation's running time can be analyzed by counting primitive steps under a declared computational model. The asymptotic upper bound T(n)=O(f(n)) means there exist positive constants C and n₀ such that T(n) ≤ C f(n) for every n≥n₀. Omega similarly states a lower bound, T(n)=Ω(f(n)), and Theta means both hold, T(n)=Θ(f(n)). These are relationships between functions as input size grows, not timing measurements in milliseconds. They also do not capture allocation patterns, cache misses, branch behavior or GPU synchronization.

A single operation often has multiple relevant input sizes. Building an index of n assets may depend on the total bytes B of their names; exploring a graph depends on vertices V and edges E; an open-addressed hash table's expected probe length depends on load factor α=n/m as well as on n; a matrix graph representation costs Θ(V²) storage even when the edge count is small. When a paper or tutorial says simply "linear time," ask *linear in what?* The same operation might be O(n) in object count but O(B) in total input bytes.

**Best-, average- and worst-case** statements require different assumptions. A linear search for a key at the front may take one comparison; its worst case scans n elements; an average-case result needs a distribution over positions or inputs. Hash lookup is expected O(1) only under a stated hashing, table and load regime; malicious collisions or pathological clustering can create much worse behavior. A balanced search tree offers O(log n) worst-case search but may have less favorable constants and locality. These distinctions do not license omitting a worst-case failure or calling an empirical benchmark "a proof of average complexity."

Space complexity should identify auxiliary storage separately from input and output storage. Merge sort may allocate Θ(n) temporary storage; in-place heapsort can use O(1) extra array space with an unstable order; recursive depth-first search may use O(V) call-stack frames in a long chain, while an explicit stack makes that usage visible and avoids stack overflow from unbounded recursion. In an engine, the difference between transient scratch and persistent storage also affects frame arenas, thread-local resources and GPU memory budgets.

### Why a vector can grow cheaply even though reallocation is expensive

A dynamic array stores elements contiguously and supports amortized O(1) append under geometric growth. Suppose capacity doubles whenever it fills. At growth sizes 1,2,4,8,..., the number of elements moved over n insertions is bounded by a geometric series:

\[
1+2+4+\cdots+2^k < 2^{k+1}\leq 2n
\]

for the relevant k near the final capacity. Although one particular insertion can move Θ(n) elements, the total relocation work across n insertions is O(n), making average amortized relocation cost O(1) per insertion. This is an **aggregate analysis**, not a guarantee that every append has constant latency. A real engine may need to reserve capacity before a frame deadline to avoid an occasional large allocation/copy in a hot path.

The same result can be derived with a potential function: associate stored potential with unused capacity or earlier cheap operations so that an expensive reallocation is paid for by the accumulated potential. The precise potential must be defined so that amortized cost equals actual cost plus the change in potential and total potential does not produce an unearned negative lower bound. This accounting is a proof technique, not a runtime counter.

When vector storage moves, pointers, references and iterators to its old elements can be invalidated. This is exactly why the M018 catalog uses logical handles instead of raw addresses. A sparse set or indirection table can preserve **logical identity** while changing dense physical storage, but it must update its index map and generations correctly. Reserving enough capacity can reduce reallocations, not fix references that were already invalidated before reserve or create identity guarantees absent from the design.

A linked list avoids relocating existing nodes during insertion at a known position, but the nodes commonly require separately allocated memory and pointer chasing. A singly linked list needs a previous node to delete an arbitrary element efficiently; a doubly linked list stores both previous and next pointers to support local bidirectional edits. An **intrusive list** embeds links inside the caller-owned objects, saving a separate wrapper allocation but imposing a strict lifetime and membership contract, as M018 demonstrated. A queue of render commands may benefit from contiguous or ring-buffer storage even when a list offers the same O(1) logical enqueue/dequeue cost.

### A ring queue is the first container where a boundary bug becomes a temporal bug

An asset-streaming queue receives new work and processes it in FIFO order. We can represent it with a fixed array, a head index and a count. Its invariant is 0 ≤ count ≤ capacity; the next insertion position is

\[
\mathrm{tail}=(\mathrm{head}+\mathrm{count})\bmod C.
\]

When count=C, the queue is full; when count=0, it is empty. This avoids the ambiguity of a representation using only head and tail, for which head==tail can mean full *or* empty unless one slot is deliberately left unused or another state bit/count is stored. A design that leaves one slot unused has effective capacity C−1. Neither representation is universally better; the full/empty policy must be explicit to callers.

The lab implements a **Ring<int,3>** with optional elements, pushes 10,20,30, rejects a fourth push, pops two entries, inserts 40,50 after wraparound and confirms the remaining FIFO sequence 30,40,50. That trace proves the tested count and modulo behavior, not thread-safety. A single-producer/single-consumer lock-free ring additionally needs atomic index publication, release/acquire ordering, careful handling of capacity and wrap and a clear ownership contract for each slot. A multi-producer/multi-consumer ring is more complicated still. Dropping a mutex because array indexing is simple would be a category error.

Other queue and stack representations differ. A stack returns last-in-first-out (LIFO), making it useful for explicit DFS or nested temporary lifetimes. A deque supports efficient operations at both ends and often uses segmented storage rather than one contiguous vector; iterator and reference stability contracts differ from vector and must be checked. Scheduling queues may use one priority heap, per-worker double-ended work-stealing deques or separate lanes for latency-sensitive tasks. The queue's ADT contract should reflect its intended ordering, not assume every FIFO is suitable for parallel scheduling.

### Hash tables trade a comparison tree for a probe invariant

The editor's name-to-handle index must answer "is this asset name present?" and "what handle does it map to?" A hash function maps a key to a table bucket. Collisions happen when different keys share that bucket. How we resolve them determines both correctness and locality.

**Separate chaining** stores a collection of keys at each bucket, often with nodes or small arrays. Inserting a collision adds to the bucket's collection; lookup scans the chain. The expected cost depends on bucket distribution and load, but linked chains can be cache-unfriendly. **Open addressing** stores entries directly in an array and probes a sequence of alternative positions. Linear probing tries consecutive slots; quadratic probing uses a growing offset sequence; double hashing uses a second hash to set a step. Each has conditions needed to guarantee that a free slot can be found and that the search sequence matches insertion's sequence.

The Phase 5 lab implements an actual **linear-probing** table. At each candidate position there are three logical states: never occupied, currently occupied, or formerly occupied and now deleted. The third state is a *tombstone*. Lookup may stop at a never-used slot because a key inserted under the same probe sequence could not have passed that state. It must **not** stop at a tombstone: the key may lie farther along the collision chain. This is a subtle but essential invariant.

Insertion searches for an existing key before deciding to reuse an earlier tombstone. If it inserted immediately into the first tombstone, it could create a duplicate entry even though the same key existed farther down the probe chain. The implementation remembers the first tombstone while probing; if it discovers an existing key it updates the value, and otherwise inserts into an available slot. Deletion changes an occupied slot into a tombstone without breaking later searches. Rehashing allocates a new table and reinserts live records into their new probe positions; all direct pointers into the old table would be invalidated. The tested implementation is an illustrative integer-key table, not a hardened security-sensitive hash map or a production-quality exception-safe generic container.

The **load factor** α=n/m measures live entries relative to bucket count. Probe lengths generally rise as the table becomes crowded; tombstones create an additional effective occupancy pressure. Resizing lowers occupancy and rebuilds probe chains but costs Θ(n) at the moment of rehash, even though geometric resize policies can make insertion amortized efficient. Robin Hood hashing stores or derives probe distances and may swap an incoming key with a key having a shorter probe history, seeking to reduce the variance of lookup lengths; that is a separate algorithm, not the same thing as ordinary linear probing. A robust explanation needs to derive insertion, successful search, unsuccessful search, deletion and resize invariants individually.

A **perfect hash** scheme seeks collision-free placement for a known, fixed key set, often after preprocessing and with constraints on changing the set. It is not a default dynamic-map replacement. It can be attractive for immutable renderer lookup tables or generated keyword sets, while ordinary dynamic asset names require insertion/removal semantics. The correct choice depends on whether the key set changes and whether memory/build cost is acceptable.

### A Bloom filter is a one-sided answer to a different question

Sometimes the streaming system wants to ask whether a requested asset is *definitely not present* before performing an expensive lookup. A Bloom filter uses a bit array of length m and k hash functions. Adding a key sets k selected bits. Querying a key checks those bits; if any is zero, the key was never added **under the filter's insertion and no-deletion contract**. If every bit is one, the key is only *possibly* present because other inserted keys may have set the same bits. Hence ordinary Bloom filters permit false positives but not false negatives for inserted keys, assuming the bits were not cleared and the hashing representation is consistent.

With n inserted keys and ideal independent uniform hashing assumptions, a familiar approximate false-positive probability is

\[
p_{\mathrm{fp}}\approx\left(1-e^{-kn/m}\right)^k.
\]

The approximation comes from estimating the probability that a bit remains zero after kn bit assignments as (1−1/m)^{kn}≈e^{-kn/m}, then requiring all k queried positions to be one. This is an approximate probabilistic model, not an exact guarantee for arbitrary poor or correlated hash functions. The choice of m and k trades memory, compute and false-positive rate.

The lab sets two bit positions for each inserted integer and verifies that every inserted value still reports *possibly present*. It intentionally does not assert that absent keys always report false, because that would contradict the data structure's contract. To support deletion, a counting Bloom filter or another carefully designed variant is needed; clearing bits in a basic filter can create false negatives for other keys that share them.

A **bitset** itself is a dense representation of boolean membership over a bounded integer domain. If our engine tracks a known finite set of component flags, contiguous bits can enable efficient set union/intersection via word-wise operations. However, a huge sparse identifier space wastes memory if directly indexed. A Bloom filter trades exact answers for compact approximate membership; a bitset represents exact membership in its declared universe; a hash set represents exact keys in a dynamic universe. These are distinct ADT contracts.

### Trees emerge when the question changes from existence to ordered structure

A hash table is poorly suited to "find all assets whose names are between A and M" or "give me the next key after this one" without additional ordering. A **binary search tree (BST)** maintains the invariant that keys in the left subtree are ordered before the current key and those in the right subtree after it. Search follows comparisons from root to a child until the key is found or an empty child is reached. Its time is O(h), where h is the tree height. A balanced tree keeps h=O(log n), while an ordinary unbalanced BST can degenerate into a chain with h=Θ(n) under sorted insertion.

**AVL trees** maintain a bound on each node's left/right subtree height difference and rebalance using single or double rotations. A rotation changes local parent-child links while preserving in-order key order. The algorithm must update stored heights and parent pointers correctly after a structural change; forgetting one can turn an apparently valid tree into an incorrect future insertion or deletion. **Red-black trees** enforce coloring constraints, a black root and NIL leaves, no red-red parent/child relation and equal black-height along root-to-leaf paths. These invariants constrain the longest path relative to the shortest and yield logarithmic height, although their exact balancing and update costs differ from AVL trees. Choosing AVL vs red-black is a tradeoff across lookup frequency, mutation frequency, memory and implementation invariants, not a matter of one being universally "faster."

A **B-tree** generalizes a node to hold multiple sorted keys and child pointers, reducing tree height by increasing branching factor. **B+ trees** commonly store all actual records or payload references in leaves, with internal nodes guiding searches and linked leaves supporting range iteration. Their strength appears when a node maps naturally to a page or cache block; asymptotic logarithms in the number of keys are less informative than the number of blocks touched. Disk-oriented indexes, memory-mapped scene metadata and on-disk package catalogs motivate this model. A single binary node per allocation can be a poor fit when an expensive storage access retrieves an entire page anyway.

A **trie** indexes keys by successive symbols or bits, with branching along the key prefix. A radix tree compresses chains of single-child nodes into labeled edges or larger steps, reducing pointer overhead when paths share long prefixes. The cost depends on key length and representation, not just the number of keys. Tries can support prefix queries and routing; their pointer-heavy variants may waste memory for sparse alphabets. Packed radix nodes or compressed tries can improve locality with different mutation complexity.

These balanced and prefix trees are developed here through their invariants and usage contracts. The current Phase 5 laboratory **does not implement AVL rotations, red-black deletion, B-tree splitting/merging or radix-tree insertions**. Completing those as independently tested production-quality containers would require separate implementations with adversarial insertion, deletion, iteration, node lifetime and randomized oracles. The curriculum map explicitly marks this distinction rather than promoting topic mention into executable proof.

### A heap is an ordering promise weaker than a sorted array

When the navigation or task system needs the next smallest tentative distance, a **priority queue** is the relevant ADT. A binary heap implements it with an array representing a nearly complete binary tree. With zero-based indexing, a node at index i has children at 2i+1 and 2i+2 when those indices are in bounds; a non-root node's parent is floor((i−1)/2). The min-heap invariant requires each parent key to be no greater than its children. It says nothing about the relative order of siblings or unrelated branches, so simply iterating the array does not produce sorted output.

Insertion appends a new element and **sifts up** while it violates the parent ordering. Removal of the minimum replaces the root with the last element, removes that element and **sifts down** until the heap property is restored. Both take O(log n) swaps/comparisons in the worst case because a complete tree has logarithmic height. Accessing the minimum is O(1), building a heap bottom-up is O(n), and a binary heap can be used for heapsort or as the backend of a priority_queue. Heapsort runs O(n log n) with O(1) array auxiliary storage in a typical in-place implementation but is generally not stable.

The lab's Dijkstra implementation uses a standard-library binary priority queue rather than reimplementing heap operations. It pushes improved distances and skips stale entries when an old queue entry no longer matches the current best distance. This avoids requiring a decrease-key operation; it can increase heap size and must be included in complexity and memory analysis. It also makes an observable distinction between **the priority queue's operational order** and the **graph algorithm's correctness invariant**: an implementation may extract entries in priority order but still return incorrect paths if its edge-weight assumptions are violated.

### Disjoint sets explain connectivity that repeatedly changes

The broadphase may group colliders into contact islands, a graph algorithm may build a minimum spanning tree, and a simulation may merge connected components. We often need to maintain a partition of elements into disjoint sets under **find** ("which group contains this element?") and **union** ("merge these two groups"). A disjoint-set union (DSU) structure stores a parent forest. Each root represents one set. **Union by size or rank** attaches the smaller/shallower tree under the larger/greater-ranked root. **Path compression** shortens a find path by redirecting visited nodes toward the root.

The lab performs two unions connecting nodes 0,1,2; verifies that trying to connect 0 and 2 again does not merge a new component; and confirms node 4 remains separate. It validates the partition semantics and index bounds. With both union by rank/size and path compression, a sequence of m operations on n elements has a near-linear amortized bound O(m α(n)) under the standard analysis, where α is the inverse Ackermann function and grows extraordinarily slowly. That bound is not "each call constant time in the strict worst case"; an individual find can still traverse a path before compression. Path compression modifies parents even during find, which complicates naive concurrent use: this sample is single-threaded.

DSU works naturally for *adding* connectivity and asking whether two vertices are connected. It does not support arbitrary edge deletion with the same basic operations, because removing one edge may split a component and cannot be represented by undoing a parent pointer without tracking more structure. Dynamic connectivity, rollback DSU and offline processing are distinct problems. For physics solver islands, a union-find pass can group interacting bodies in a frame, but stable island wake/sleep policies and incremental contact events need additional state.

The group-selection lesson is now visible: arrays, rings, hash tables, trees, heaps and DSU each solve different semantic queries. Their asymptotic bounds explain growth; Phase 4 memory reasoning explains why implementations with identical bounds can behave differently; and source/lifetime constraints explain whether the chosen representation is even safe for an editor or renderer. This is the M025–M027 authored foundation, not a claim that every specialized tree has already been engineered and benchmarked.

**Independent-source comparison:** MIT's [6.006 Introduction to Algorithms, Spring 2020 lecture recordings](https://ocw.mit.edu/courses/6-006-introduction-to-algorithms-spring-2020/video_galleries/lecture-videos/) include dynamic arrays, hashing, trees, heaps and graph foundations, while the 2011 lectures explicitly include open addressing and table doubling. We checked the official course inventories and use them as targeted source routes; no unwatched lecture is described as fully reviewed. The supplied *Game Engine Architecture* provides an engineering context for choosing containers and memory layouts. Its older implementation choices are not automatically current C++ guarantees. Current KairoSpatial/KairoECS source files remain uninspected in this vault-only teaching addition.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/06 - Queries, lazy ranges, allocators, and module boundaries|Previous chapter]] · [[Chapter Edition/03 - Game Engine/08 - Algorithms, application boundaries, and a first playable|Next chapter]]
