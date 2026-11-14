# 05 — Generic Asset Pipelines without Hidden Contracts

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/04 - Prove C++ Ownership with Runnable Experiments|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/06 - Grow Registries without Invalidating Identity|Next chapter]]

A single loader pattern must serve meshes, textures, and other assets without erasing their different validation and ownership needs. This chapter develops a generic pipeline whose policies preserve caller intent and whose failure boundaries remain visible.

**By the end:** Build shared loader structure around type-specific validation, resource ownership, capacity, and copy/transfer policy.

**Recall question:** Which steps can be shared across asset types, and which must remain specific to an asset’s meaning?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 7108–8604; source-span SHA256 d3ef0a635e6493a87500dc691a49d29a8cf312d736efe57d1655bdbc2d5f0619. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## One asset pipeline, many asset types — generic code without accidental promises

The previous story followed a single upload from external bytes through validation, resource acquisition, private preparation and publication. It established what the loader must preserve when construction fails, who owns the native handle, and what a delayed callback may safely retain. But we now have a new practical problem: a game editor loads meshes, textures, shader descriptions and many other kinds of assets. They do not all contain the same fields or need the same validation. Yet every loader repeats the same bookkeeping: check capacity, construct a complete record, publish it only after construction succeeds, return an identifier and avoid leaking a partially prepared result.

Copying this bookkeeping into separate `MeshStore`, `TextureStore` and `ShaderStore` classes would make every later correction difficult. If we discover a failure in publication or a missing capacity check, we would have to repair several implementations and prove that they remain consistent. At the opposite extreme, using `void*` and manually interpreting bytes would throw away the type and lifetime safety we worked to establish. We need a middle path: **write the operation once, preserve the actual asset's type, and reject operations that make no sense for that type.**

This is where C++ templates become necessary. A template is not a runtime container for arbitrary objects. It is a declaration parameterized by types, values, or other template arguments; the compiler forms usable specializations when code needs them. A well-designed template lets us express a shared rule while still allowing the compiler to know whether it is operating on a `Mesh` or a `Texture<4>`. The price is that requirements, generated code, compilation cost, overload behavior and ownership guarantees must all remain explicit. We will derive them from our asset pipeline rather than presenting templates as a detached language reference.

**Working reference:** [[tools/labs/generic_asset_pipeline_lab.cpp|C++20 generic asset laboratory]]. The laboratory is a CPU-side asset preparation model with fixed optional slots, checked byte counts and deliberate failure injection. It is not a direct replacement for the Phase 2 native-resource owner, and it does not claim to execute GPU work. Our eventual engine joins the two layers: validated type-specific preparation first, a resource-owning backend transaction second.

### The repeated operation is a function whose input type can vary

Start with the question "How many source bytes does this asset represent?" A mesh stores a vector of 32-bit indices. A texture has a channel count and a texel count. For a mesh with three indices, the simplest byte-size calculation is $3\times\operatorname{sizeof}(\texttt{uint32\_t})$. On an implementation with a 32-bit exact-width unsigned integer type, this is $3\times4=12$ bytes. A four-channel texture with twelve texels needs $4\times12=48$ bytes if each stored channel is one byte; a three-channel texture with five texels needs $3\times5=15$ bytes. These calculations represent the source payload model, not GPU row padding, compression, mip chains or device allocation requirements.

We could define three differently named functions, but it would be better if an asset-aware operation could accept any supported type and use the correct byte-size rule. A **function template** expresses that shape:

```cpp
template <class T>
std::size_t asset_bytes(const T& asset) {
    return AssetTraits<T>::byte_size(asset);
}
```

The `template <class T>` declaration introduces a type parameter, meaning that `T` stands for a type selected when the function is used. Calling `asset_bytes(mesh)` does not require us to type `asset_bytes<Mesh>(mesh)` every time. During **template argument deduction**, the compiler compares the parameter pattern `const T&` with the argument expression and determines the appropriate `T`. It then checks whether the resulting specialization is a valid function. The expression `AssetTraits<T>::byte_size(asset)` is type-dependent, so its meaning is determined for the selected type rather than being a single runtime switch on an integer type code.

This is different from the preceding chapter's polymorphic `Backend`. A virtual backend can be chosen through a base pointer at runtime. Here the compiler already knows the asset type at a particular call site and can form type-specific operations. The actual machine code may be inlined or outlined, duplicated or merged, depending on compiler optimization and link behavior. A template is therefore not automatically a promise of zero instructions or one code copy for all types; its important initial benefit is that the *source rule* is written once while the selected types remain statically known.

One subtle point: deduction does not work by guessing whatever type would make the function body compile. It first follows parameter/argument deduction rules, then substitution and validity checks. An argument that is an lvalue, a const lvalue or a temporary can produce different deductions for a forwarding-reference parameter, while `const T&` usually deduces the unqualified object type for the common examples here. Some contexts, such as certain dependent nested type names, do not deduce template arguments at all. Braced initializer lists also have specific deduction rules and may fail where a named `std::vector` would succeed. We should never describe deduction as "C++ automatically figures out everything from the code"; it solves a constrained pattern-matching problem under precise rules.

### Different asset types share a store, but their meanings remain different

We now want one store class that retains a particular asset type without converting it into untyped memory. A **class template** is a family of types described by a common declaration. Consider:

```cpp
template <class T, std::size_t Capacity>
class AssetDepot {
    std::array<std::optional<T>, Capacity> slots_{};
    std::size_t count_ = 0;
    // ...
};
```

The type parameter `T` specifies the actual stored object type. The value parameter `Capacity` specifies a compile-time capacity. Unlike `T`, which is a type, `Capacity` is a **non-type template parameter**: a value that participates in defining a specialization. `AssetDepot<Mesh, 2>` and `AssetDepot<Texture<4>, 2>` are different class types. Changing the capacity also selects a different specialization; `AssetDepot<Mesh, 2>` and `AssetDepot<Mesh, 8>` are not interchangeable by ordinary class identity.

Why use a compile-time capacity in this reference laboratory? A fixed `std::array` of `std::optional` slots makes the publication invariant exceptionally clear. Before an insertion, the next slot is empty. We construct the proposed object there. Only when that construction succeeds do we increment the count of published records. If construction throws, the old published slots remain untouched. This isolates the language behavior we want to learn.

There are tradeoffs. A fixed array reserves storage for every optional slot inside the depot object, even when the store is empty. Large assets or large capacities can make that object expensive or inappropriate for stack placement; a heap-backed or paged container may be better in a real engine. Constructing `AssetDepot<Mesh, 2>` and `AssetDepot<Mesh, 8>` can also instantiate separate code, and callers cannot freely substitute one for the other. Static capacity is ideal for this *bounded demonstration*, not proof that production asset registries should never grow.

The `std::optional<T>` slot has a straightforward meaning: it either contains a live `T` object or does not. It therefore saves us from having to manually implement the earlier union's active-member tracking. We still owe the caller a clear identity and lifetime contract. The lab's `AssetId{slot}` identifies a slot only while the depot's append-only sequence remains alive. It has no generation counter, removal or reuse policy. **It is not the generational asset handle design required by a real ECS or hot-reloading engine.** If a real store deletes and reuses slots, it must add generation validation and an invalidation model rather than pretending this small example already supplies them.

### The size rule must vary with the asset's semantic type

The shared store needs a common question—"How many bytes does this asset represent?"—but not a common answer. A mesh counts indices. A texture multiplies channel count by texel count. We can separate the *policy for a type* from the shared storage operation through a class-template trait:

```cpp
template <class T>
struct AssetTraits;  // deliberately no general implementation

template <>
struct AssetTraits<Mesh> {
    static constexpr std::string_view label = "mesh";

    static std::size_t byte_size(const Mesh& mesh) {
        constexpr auto width = sizeof(std::uint32_t);
        if (mesh.indices.size() >
            std::numeric_limits<std::size_t>::max() / width)
            throw std::overflow_error("mesh size overflow");
        return mesh.indices.size() * width;
    }
};
```

The initial declaration says that `AssetTraits<T>` is a possible template family; it does **not** promise that every conceivable C++ type is supported. The `template <>` definition is an **explicit specialization** for precisely `Mesh`. It says that the shared asset protocol is meaningful for this type and supplies that type's operations. Unlike an instance-level virtual method, this policy is selected using the known type, not an object's dynamic identity.

For textures we have an entire family of related types. `Texture<3>` and `Texture<4>` carry different compile-time channel counts, but they share the same formula for byte count. Instead of writing a separate complete specialization for each channel count, use a **partial specialization**:

```cpp
template <std::size_t Channels>
struct AssetTraits<Texture<Channels>> {
    static constexpr std::string_view label = "texture";

    static std::size_t byte_size(const Texture<Channels>& texture) {
        if (texture.texel_count >
            std::numeric_limits<std::size_t>::max() / Channels)
            throw std::overflow_error("texture size overflow");
        return texture.texel_count * Channels;
    }
};
```

This matches any member of the `Texture<Channels>` type family, subject to the texture type's own permitted channel counts. The compiler determines `Channels` from the selected `Texture` type. The multiplication must still be checked at runtime, because `texel_count` comes from data and may be too large. A compile-time channel count does not remove the need for input validation.

**Explicit versus partial specialization** is a difference in how much of the template argument pattern we fix. `AssetTraits<Mesh>` fixes the complete concrete type. `AssetTraits<Texture<Channels>>` fixes the outer type family while leaving the channel count parameterized. Function templates cannot be partially specialized in the same way as class templates; for function behavior differences we commonly use overloading, constrained templates, delegation to specialized traits, or other supported design techniques. Several partial specializations can also overlap, which creates an additional ordering problem; a design must guarantee a unique most-specialized match for the types it intends to accept.

This division avoids an implementation trap. A generic store should know how to **hold, construct and publish** an asset; a trait should explain which semantic operations a supported type provides. Neither should pretend the source-byte count automatically equals device allocation size, GPU row pitch or a serialized format's total length. Those are separate policies with their own units and constraints.

### A resource store must preserve the caller's intention to copy or transfer

Our Phase 2 owner had explicit move operations. If a generic store takes an asset's constructor arguments by value, it can create unnecessary intermediate copies. If it indiscriminately applies `std::move` to everything, it can steal storage from an argument that the caller intended to retain. We need to preserve whether the original argument is an lvalue or an rvalue.

First recall the value categories through the mesh's two constructors:

```cpp
struct Mesh {
    std::vector<std::uint32_t> indices;
    bool copied_input;

    explicit Mesh(const std::vector<std::uint32_t>& input)
        : indices(input), copied_input(true) {}

    explicit Mesh(std::vector<std::uint32_t>&& input)
        : indices(std::move(input)), copied_input(false) {}
};
```

A named `std::vector` passed as an lvalue selects the borrowing `const&` constructor and makes an independent copy of the indices. A temporary or an explicitly moved vector can select the `&&` overload, allowing the vector's resources to be transferred into the mesh. The `std::move` expression itself is a cast that makes an rvalue access path available; the actual resource transfer happens in the selected constructor or assignment operation. After a move, the original vector remains valid but its exact contents are not generally a promise our algorithm should depend on.

Now consider a template written with `T&&`. **It is not always an rvalue-reference parameter** in the simple sense. When `T` is deduced for a suitable function-template parameter, `T&&` is a *forwarding reference*. Passing an lvalue of type `U` deduces `T=U&`; passing an rvalue usually deduces `T=U`. That leads us to **reference collapsing**, the rules that simplify references formed through type aliases and template substitution:

| Inner reference | Outer reference | Collapsed result |
|---|---|---|
| `U&` | `&` | `U&` |
| `U&` | `&&` | `U&` |
| `U&&` | `&` | `U&` |
| `U&&` | `&&` | `U&&` |

In ordinary language, if either side is an lvalue reference, the collapsed result is an lvalue reference; only two rvalue-reference layers collapse to an rvalue reference. This preserves a fundamental promise: an expression supplied as a named, retained object does not suddenly become permission to move from it merely because it crosses a generic function boundary. By contrast, a parameter of the form `const T&&` is **not** the same forwarding-reference pattern, and a type parameter already fixed by a class template may not be deduced in the way this function-template pattern requires.

Our insertion method uses both a **variadic template parameter pack** and forwarding references:

```cpp
template <class... Args>
    requires std::constructible_from<T, Args&&...>
AssetId emplace(Args&&... args) {
    if (count_ == Capacity)
        throw std::length_error("asset depot full");

    slots_[count_].emplace(std::forward<Args>(args)...);
    return AssetId{count_++};
}
```

The symbols `Args...` mean a sequence of zero or more template type arguments, not one special runtime type. The function receives one forwarding-reference parameter corresponding to each deduced type. `std::forward<Args>(args)` uses that deduction to produce an access expression with the same effective value category as the original caller-supplied argument. The `...` after the forwarding expression expands the pack into the arguments for `optional<T>::emplace`. This means the store can call whatever `T` constructor is valid for those argument types, without manually duplicating constructor overloads in `AssetDepot`.

**Perfect forwarding** is the common name for this preservation mechanism. The word "perfect" should not be mistaken for guaranteed zero cost, zero allocations or universally correct semantics. It preserves argument categories *subject to C++ deduction rules*; the target constructor may still allocate or copy, and some arguments have special deduction behavior. It also preserves dangerous borrows if the target object stores references to short-lived data. The target type's constructor is still responsible for adopting or copying any state it retains.

A fully worked trace makes this concrete. The lab constructs `std::vector<std::uint32_t> input{3,7,9}`. Calling `meshes.emplace(input)` deduces an lvalue reference type for its one argument, forwards it as an lvalue, and selects `Mesh(const vector&)`. The mesh records `copied_input=true`, while the source vector remains usable with three elements. Calling `meshes.emplace(std::move(input))` treats the argument as an rvalue, forwards it accordingly and selects `Mesh(vector&&)`. The second mesh records `copied_input=false` and contains three indices. We deliberately do not assert that the moved-from input vector must be empty: **its exact moved-from state is not the same portable guarantee as the destination's correct contents.**

This trace is evidence about constructor selection and ownership on the tested type. It is not an assertion that moving every asset is cheaper than copying, or that a forwarded temporary can safely be retained by a target that stores an unchecked reference.


### Modern C++ rules matter when they change ownership, lifetime or concurrency contracts

The broad C++ story in this volume is already deeper than any single “tips” book, so the useful contribution from A Tour of C++, Effective Modern C++, Professional C++, The Cherno and CppCon is not another feature catalogue. Their strongest material is the set of places where a convenient-looking language feature changes an engine's correctness or performance contract.

#### move is a cast of expression category, not a transport operation

A call such as

~~~
std::move(resource)
~~~

does not itself move bytes or transfer ownership. It produces an xvalue expression that allows overload resolution to select move-aware operations.

The actual move happens only if the receiving operation implements one.

This explains why

~~~
const Texture texture = ...;
consume(std::move(texture));
~~~

may still copy: most move constructors take a non-const rvalue reference because moving usually modifies the source. A const rvalue cannot bind to that parameter, so a copy overload may win.

The engine rule is:

\[
\boxed{
\text{use move to express permission to transfer from a source whose state may change}
}
\]

not “sprinkle move on expensive objects.”

#### forwarding references preserve caller category only with forwarding

For a template

~~~
template<class T>
void add(T&& value);
~~~

the parameter can be a forwarding reference when T is deduced. If the caller passes an lvalue, T deduces as an lvalue reference; if the caller passes an rvalue, T is a non-reference type.

Inside the function, however, the named variable value is itself an lvalue expression. Passing value onward therefore loses the caller's category unless the function uses forwarding:

~~~
target(std::forward<T>(value));
~~~

This is why an asset factory or ECS construction helper can accidentally turn every rvalue into a copy even when its public API looks perfectly generic.

Reference collapsing provides the mechanism:

\[
T\&\& \text{ with } T=U\&
\quad\Rightarrow\quad
U\&.
\]

The engineering point is not memorizing the collapse table. It is understanding that generic wrappers must preserve the semantic difference between “borrow/copy this existing object” and “you may consume this temporary.”

#### noexcept can change which operation a container is willing to use

A move constructor that may throw can force a standard container to prefer copying during reallocation when copying offers a stronger exception guarantee.

So

~~~
Type(Type&&) noexcept;
~~~

is not merely documentation if the operation truly cannot throw. It can affect whether vector growth moves or copies elements.

But declaring noexcept dishonestly is worse: an escaping exception causes termination.

The rule is therefore:

\[
\boxed{
\text{declare noexcept when the implementation's contract genuinely supports it}
}
\]

and understand that exception specifications participate in optimization and generic-library behavior.

#### special-member generation is an ownership audit

Once a type manually declares some combination of destructor, copy constructor, copy assignment, move constructor or move assignment, implicit generation rules change.

For an engine resource wrapper this is not trivia. A handle type that owns a GPU buffer should answer explicitly:

~~~
copyable?
movable?
destructor owns release?
moved-from state valid?
copy duplicates ownership or shares it?
~~~

If copying would create two owners of one raw handle, delete copying deliberately.

If moving transfers the handle, leave the source in a destructible empty state.

A type's special members are therefore a compact statement of its ownership algebra.

#### const member functions are not automatically thread-safe

A const member function promises not to modify the object's logical state through the ordinary non-mutable interface. It does not create synchronization.

A cache may be mutable. A lazily initialized field may be mutable. Another object reachable through a pointer may change. Two threads may call the same const method and still race.

For a shared engine object, the concurrency contract must be separate:

\[
\boxed{
\text{const-correctness}
\neq
\text{data-race freedom}
}
\]

If a const method mutates a cache, that cache needs synchronization or a design that avoids shared mutation.

#### volatile is not a threading primitive

In C++, volatile primarily constrains optimization around accesses intended for special memory or externally observable interactions. It does not make an operation atomic and does not establish inter-thread happens-before relationships.

For inter-thread synchronization use the concurrency model:

- atomics,
- mutexes,
- condition variables,
- futures/tasks,
- or higher-level queues/job systems.

For hardware/MMIO or other special memory, volatile may be relevant according to platform rules.

The dangerous sentence is:

> this flag is volatile, so the worker thread will see it.

That is not a valid C++ synchronization argument.

#### atomics solve one memory location; they do not design the protocol

Making a state variable atomic prevents a data race on that atomic object. It does not automatically make surrounding non-atomic data safe.

A publication pattern needs an ordering relationship:

~~~
producer writes payload
producer publishes readiness with release

consumer observes readiness with acquire
consumer reads payload
~~~

If the protocol uses relaxed atomics, it must prove that only atomicity/modification order is required.

The memory order is part of the algorithm. Choosing memory_order_relaxed because it “sounds fast” is not performance engineering.

#### default lambda capture hides lifetime dependencies

A lambda that captures by reference can outlive the referenced stack object. A lambda that captures this implicitly can outlive the object. A broad capture mode can also silently begin capturing a newly referenced variable after later edits.

For callbacks, jobs and deferred GPU/IO completions, prefer explicit capture because lifetime is part of correctness.

Badly hidden dependency:

~~~
enqueue([&] { upload(asset); });
~~~

Better design asks:

- does the job own asset?
- borrow it under a larger lifetime?
- copy an ID and resolve later?
- move a self-contained payload?

The capture list should reveal that answer.

#### task-based APIs describe intent better than manually owning a thread

A task says “run this work and produce/completion-signal this result.” A thread says “own this operating-system execution resource.”

Engine code usually cares about the first.

That is why futures, task systems and job graphs can scale better architecturally than one subsystem creating a permanent thread for every asynchronous need.

The caveat is that standard async facilities have policy/lifetime details of their own. A production engine still needs to know whether work is guaranteed asynchronous, how exceptions propagate, how cancellation works, and how tasks interact with the engine scheduler.

#### emplacement is not automatically faster or safer

Emplace-style APIs construct an element from arguments at the destination. That can avoid an intermediate temporary, but the benefit depends on the call.

If the caller already has an object, insertion plus move may be equally appropriate. Emplacement can also enable conversions that an explicit temporary would have made visible.

Use emplacement when constructing the element from constructor arguments is the intended operation, not because “emplace is the modern faster spelling of insert.”

#### explicit modern C++ is often simpler than clever modern C++

The consistent message across the modern C++ sources is not “use the newest feature everywhere.” It is:

\[
\boxed{
\text{make ownership, lifetime, type intent and synchronization easier to verify}
}
\]

A Tour of C++ gives the integrated language map. Effective Modern C++ concentrates on C++11/14 feature interactions such as type deduction, smart pointers, move/forwarding, lambdas and atomics. Professional C++ broadens the discussion into design, testing, debugging, efficiency, multithreading and maintainability. CppCon talks are most useful when a particular engine problem needs a deeper specialist treatment. The Cherno's C++ material gives practical demonstrations that can be connected back to these rules.

The source hierarchy for this volume is therefore:

\[
\text{language semantics}
\rightarrow
\text{small example}
\rightarrow
\text{engine ownership/lifetime consequence}
\rightarrow
\text{measured production behavior}
\]

rather than a list of style commandments.



### CppCon deepening: engine C++ is mostly about proving lifetime, layout, and execution contracts

The previous modern-C++ section establishes the language mechanics. The next step is to connect those mechanics to failures that appear only when a codebase becomes large, asynchronous, data-heavy, and performance-sensitive. Recent CppCon material repeatedly returns to the same production lesson: the hardest C++ problems are rarely solved by memorizing more syntax. They are solved by making hidden contracts visible enough that the compiler, profiler, sanitizer, reviewer, and runtime can all check them.

#### Rule of zero is a reduction in the number of states you must prove correct

Suppose an engine resource wrapper manually owns a heap allocation:

~~~cpp
class CpuBlob {
public:
    explicit CpuBlob(std::size_t bytes)
        : size_(bytes),
          data_(new std::byte[bytes]) {}

    ~CpuBlob() {
        delete[] data_;
    }

private:
    std::size_t size_{};
    std::byte* data_{};
};
~~~

This type already has a problem: the compiler-generated copy constructor copies the pointer value, so two objects can later delete the same allocation.

The naive response is to implement every special member manually. That can be correct, but it expands the proof surface. Copy construction, copy assignment, move construction, move assignment, destruction, self-assignment, exception paths, and moved-from state all require review.

A better representation is:

~~~cpp
class CpuBlob {
public:
    explicit CpuBlob(std::size_t bytes)
        : data_(std::make_unique<std::byte[]>(bytes)),
          size_(bytes) {}

    std::span<std::byte> bytes() noexcept {
        return {data_.get(), size_};
    }

    std::span<const std::byte> bytes() const noexcept {
        return {data_.get(), size_};
    }

private:
    std::unique_ptr<std::byte[]> data_;
    std::size_t size_{};
};
~~~

Now ownership semantics are inherited from a tested vocabulary type. The compiler generates destruction and moving correctly, while copying is disabled because the owner is non-copyable.

The important reduction is

\[
\boxed{
\text{fewer custom lifetime transitions}
\Rightarrow
\text{fewer states requiring manual proof}
}
\]

Rule of zero therefore matters to engine design because it moves correctness responsibility from ad-hoc transitions into well-defined types.

#### Value, owner, observer, and handle are different categories even when their representations are similar

A pointer, integer handle, unique owner, span, and index can all be small machine values, but their semantic contracts are completely different.

A useful engine vocabulary is:

~~~text
value:
    conceptually owns its value state
    copying creates another value

owner:
    controls a resource lifetime
    destruction has release semantics

observer:
    refers to state owned elsewhere
    must not outlive that owner

stable handle:
    indirect identity that can be re-resolved
    can survive relocation of the underlying object

ephemeral index:
    valid only relative to a particular container epoch
~~~

This distinction prevents bugs that cannot be diagnosed from the raw bit pattern alone.

This is dangerous:

~~~cpp
struct TextureJob {
    Texture* texture;
    UploadQueue* queue;
};
~~~

The two pointers hide whether either object is owned, borrowed, nullable, stable, or allowed to outlive the scheduling scope.

A stronger interface can carry stable identity and explicitly owned payload:

~~~cpp
struct TextureId {
    std::uint32_t slot{};
    std::uint32_t generation{};
};

struct TextureUploadJob {
    TextureId texture;
    std::shared_ptr<const UploadPayload> payload;
};
~~~

The job owns the immutable upload payload but only carries identity for the texture. When the job executes, it resolves the handle through the resource manager. If the generation no longer matches, the object was destroyed or recycled and the job can fail safely instead of dereferencing a stale pointer.

The rule is:

\[
\boxed{
\text{representation should reveal lifetime category whenever practical}
}
\]

A raw pointer can be correct, but pointer alone does not communicate enough of the lifetime contract for long-lived asynchronous engine work.

#### Moved-from is a valid-state requirement, not a promise that the old value stays meaningful

Move semantics are often described as making the source empty. The language contract is generally weaker: a moved-from object must remain valid enough for the operations its type promises, especially destruction and assignment, unless a stronger postcondition is documented.

For a GPU resource wrapper, defining a stronger moved-from state is valuable:

~~~cpp
class BufferHandle {
public:
    BufferHandle() = default;

    BufferHandle(VkBuffer buffer, VkDevice device) noexcept
        : buffer_(buffer), device_(device) {}

    BufferHandle(BufferHandle&& other) noexcept
        : buffer_(std::exchange(other.buffer_, VK_NULL_HANDLE)),
          device_(std::exchange(other.device_, VK_NULL_HANDLE)) {}

    BufferHandle& operator=(BufferHandle&& other) noexcept {
        if (this == &other) return *this;

        release();

        buffer_ = std::exchange(other.buffer_, VK_NULL_HANDLE);
        device_ = std::exchange(other.device_, VK_NULL_HANDLE);
        return *this;
    }

    ~BufferHandle() {
        release();
    }

    BufferHandle(const BufferHandle&) = delete;
    BufferHandle& operator=(const BufferHandle&) = delete;

private:
    void release() noexcept {
        if (buffer_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
            vkDestroyBuffer(device_, buffer_, nullptr);
        }

        buffer_ = VK_NULL_HANDLE;
        device_ = VK_NULL_HANDLE;
    }

    VkBuffer buffer_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE};
};
~~~

Here exchange extracts the old value and writes a known neutral value into the source. The lifetime-sensitive type now has a small state machine:

~~~text
empty:
    buffer = null
    device = null

owning:
    buffer != null
    device != null

move:
    destination becomes owning
    source becomes empty

destroy:
    owning -> release -> empty
    empty -> no-op
~~~

If the state machine cannot be explained this simply, the type may be carrying too many responsibilities.

#### Undefined behavior removes assumptions under which optimization is reasoned about

Undefined behavior is not merely an unusual runtime result. C++ compilers optimize under the assumption that the program obeys language preconditions. Once code violates one of those preconditions, the optimizer is no longer required to preserve the intuitive behavior of the invalid execution.

This matters for:

- out-of-bounds access,
- use-after-free,
- invalid pointer provenance,
- signed overflow,
- data races,
- misaligned typed access,
- aliasing violations,
- invalid object lifetime after storage reuse,
- calling through an incompatible function type.

The engine rule is not to fear low-level code. It is to write low-level code inside a model whose assumptions can be established.

\[
\boxed{
\text{only optimize inside a contract whose preconditions you can prove}
}
\]

When code intentionally depends on representation, use mechanisms that make the operation legal and visible: bit-casting, byte copies, alignment-aware allocation, explicit lifetime APIs where needed, and small wrappers around platform-specific behavior.

Sanitizers matter because many lifetime and UB bugs stay visually silent until optimization level, workload, compiler, driver, allocator, or scheduling changes.

#### Data-oriented design begins with the loop, not with a fashionable container

Recent CppCon data-oriented material is valuable because it reframes DOD as a question about access patterns.

Suppose an engine updates one million particles:

~~~cpp
struct Particle {
    Vec3 position;
    Vec3 velocity;
    Vec4 color;
    float lifetime;
    std::uint32_t textureIndex;
    std::uint32_t flags;
};
~~~

If simulation touches only position, velocity, and lifetime, every cache line also carries render-only color, texture index, and flags.

An SoA representation could be:

~~~cpp
struct ParticleSet {
    std::vector<Vec3> positions;
    std::vector<Vec3> velocities;
    std::vector<float> lifetimes;

    std::vector<Vec4> colors;
    std::vector<std::uint32_t> textureIndices;
    std::vector<std::uint32_t> flags;
};
~~~

The simulation loop becomes:

~~~cpp
void simulate(ParticleSet& p, float dt) {
    const std::size_t n = p.positions.size();

    for (std::size_t i = 0; i < n; ++i) {
        p.velocities[i].y += -9.81f * dt;
        p.positions[i] += p.velocities[i] * dt;
        p.lifetimes[i] -= dt;
    }
}
~~~

This is not automatically faster. It helps only if memory traffic, vectorization, branch behavior, or working-set size improve on the target hardware.

A simple decomposition is:

\[
T
\approx
T_{\text{memory}}
+
T_{\text{compute}}
+
T_{\text{branches}}
+
T_{\text{synchronization}}
+
T_{\text{overhead}}.
\]

Changing AoS to SoA primarily attacks memory traffic and vectorization constraints. If another term dominates, the speedup may be small.

The real workflow is therefore:

~~~text
identify hot loop
    ↓
list fields actually touched
    ↓
measure working set/cache behavior
    ↓
change layout
    ↓
benchmark the same workload
    ↓
inspect counters/generated code if needed
~~~

This is much stronger than the slogan that SoA is always faster than OOP.

#### Physical layout and abstraction boundary do not have to be the same thing

A common false choice is clean abstraction versus fast data.

A value-like view can sit on top of SoA storage:

~~~cpp
class ParticleView {
public:
    ParticleView(ParticleSet& set, std::size_t i)
        : set_(&set), i_(i) {}

    Vec3& position() noexcept { return set_->positions[i_]; }
    Vec3& velocity() noexcept { return set_->velocities[i_]; }
    float& lifetime() noexcept { return set_->lifetimes[i_]; }

private:
    ParticleSet* set_;
    std::size_t i_;
};
~~~

Hot loops can still traverse direct arrays, while editor, debugging, scripting, and gameplay code can receive structured views.

\[
\boxed{
\text{abstraction boundary need not equal physical memory layout}
}
\]

This is central to ECS, animation, particles, transforms, visibility, and renderer scene databases.

#### span and mdspan separate ownership from access geometry

A span expresses a borrowed contiguous sequence without taking ownership:

~~~cpp
void upload_vertices(std::span<const Vertex> vertices);
~~~

The caller may own those vertices in a vector, fixed array, memory-mapped file, or custom arena. The upload function receives only the range contract.

For multidimensional data, mdspan extends the same idea to shape, strides, and index mapping. This is valuable for matrices, tiled images, tensors, simulation grids, and staging views because it separates four concerns:

\[
\text{owner}
\quad
\text{shape}
\quad
\text{layout mapping}
\quad
\text{element access}.
\]

Conceptually:

\[
\text{logical index}
\overset{\text{mapping}}{\longrightarrow}
\text{linear offset}
\overset{\text{accessor}}{\longrightarrow}
\text{element}.
\]

That separation mirrors GPU resource-view design and is useful far beyond the standard library type itself.

#### PMR matters when allocation policy is part of the subsystem contract

Polymorphic memory resources let ordinary standard containers allocate from a subsystem-specific resource.

A per-frame scratch allocator is a natural example:

~~~cpp
struct FrameScratch {
    std::array<std::byte, 2 * 1024 * 1024> backing;

    std::pmr::monotonic_buffer_resource arena{
        backing.data(),
        backing.size()
    };
};

void build_visibility_list(
    FrameScratch& scratch,
    std::span<const RenderItem> items)
{
    std::pmr::vector<std::uint32_t> visible{&scratch.arena};
    visible.reserve(items.size());

    for (std::uint32_t i = 0; i < items.size(); ++i) {
        if (is_visible(items[i])) {
            visible.push_back(i);
        }
    }

    submit_visible(visible);
}
~~~

A monotonic resource is efficient when individual deallocation is unnecessary and the whole region can be reset together.

But the critical rule is:

\[
\boxed{
\text{an object allocated from a short-lived resource must not escape that resource lifetime}
}
\]

PMR does not eliminate lifetime reasoning. It makes the allocation domain explicit enough that the lifetime can be audited.

#### Coroutines are suspension syntax; they do not automatically provide structured lifetime

A coroutine can suspend while retaining references to objects that later disappear.

~~~cpp
Task<void> stream_texture(Texture& texture) {
    auto bytes = co_await read_file_async(texture.source_path());
    co_await upload_async(texture, bytes);
}
~~~

The linear syntax hides several contracts:

- who owns texture while the coroutine is suspended,
- who owns the coroutine frame,
- what cancels the task if the texture unloads,
- where execution resumes,
- how exceptions propagate,
- whether shutdown waits for children,
- whether one child can outlive its scheduling subsystem.

Structured concurrency treats asynchronous work as a lifetime tree:

~~~text
AssetSystem lifetime
|
+-- texture streaming scope
    |
    +-- file read task
    +-- transcode task
    +-- GPU upload task
~~~

When the parent scope ends, children are completed, cancelled, or joined according to an explicit policy.

That gives a powerful invariant:

\[
\boxed{
\text{async lifetime should be nested whenever possible}
}
\]

Detached work is sometimes necessary, but it should be exceptional because detached lifetime is harder to prove.

#### Cancellation is a protocol, not a boolean checked once

A robust asynchronous pipeline needs cancellation points where it can stop without violating resource invariants.

A texture pipeline might be:

~~~text
queued
→ reading
→ decoded
→ staging allocation
→ GPU copy submitted
→ ready
~~~

Cancellation before GPU submission can usually release CPU resources immediately.

Cancellation after submission is different. GPU work may already reference staging memory, command storage, descriptor state, or the destination image. At that point cancellation often means stop publishing the logical result while allowing already-submitted work to retire safely.

Therefore:

\[
\boxed{
\text{cancel logical operation}
\neq
\text{instantly destroy all physical resources involved}
}
\]

This is the same lifetime separation used by deferred GPU destruction.

#### Build-time cost is part of architecture

Large C++ engines can lose enormous developer time to translation-unit fan-out, template instantiation, and public-header inclusion.

The runtime dependency graph and compile dependency graph are different.

A class can need a type at runtime without requiring that type's full definition in a public header. Forward declarations, pImpl, modules, implementation-only includes, stable interfaces, and disciplined template placement can reduce rebuild radius.

The relevant productivity model is:

\[
T_{\text{developer loop}}
=
T_{\text{edit}}
+
T_{\text{incremental compile}}
+
T_{\text{link}}
+
T_{\text{launch/test}}.
\]

If a frequently edited header forces hundreds of translation units to rebuild, it is a productivity hotspot even when runtime performance is perfect.

Build profiling is therefore performance engineering for the developer loop.

#### Preserve the causal chain when optimizing

A production optimization should be explainable as:

~~~text
measured symptom
    ↓
identified limiting resource
    ↓
specific hypothesis
    ↓
targeted code/data change
    ↓
same-workload remeasurement
    ↓
regression guard
~~~

For example:

~~~text
symptom:
    transform update = 3.8 ms

evidence:
    LLC misses high
    SIMD utilization low

hypothesis:
    mixed component layout streams unused data

change:
    split hot transform arrays from cold metadata

result:
    transform update = 1.9 ms
    memory traffic reduced

guard:
    benchmark scene plus performance threshold in CI
~~~

This keeps optimization from becoming folklore.

The combined rule for this C++ layer is:

\[
\boxed{
\text{make ownership explicit}
+
\text{make access patterns measurable}
+
\text{make async lifetime nested}
+
\text{make performance claims reproducible}
}
\]

Source integration basis for this deepening includes CppCon material on lifetime management, undefined behavior, data-oriented design, mdspan, performance engineering, build profiling, and coroutine/structured-concurrency practice. The conference boundaries are intentionally not preserved: each concept is placed where it constrains engine code.


### A collection of assets makes parameter packs and fold expressions necessary

The editor may submit several kinds of resources in one batch. We want the total source-byte count before deciding whether a batch fits an upload budget. A fixed function accepting one mesh and two texture arguments would not scale to other combinations. We already have `AssetTraits<T>::byte_size` for each supported type; we need a way to repeat that operation for every asset in an arbitrary argument list.

A **parameter pack** represents the type and value arguments of this variable-length compile-time input. A **fold expression** combines them using a chosen operator. For a pure addition, one might write `(std::size_t{0} + ... + asset_bytes(assets))`. But this omits a real correctness condition: accumulating large `std::size_t` values can wrap because it is unsigned, quietly producing a smaller total. Source-byte accounting that determines whether an allocation fits must not ignore overflow.

The tested lab therefore folds over a checked addition operation:

```cpp
template <Uploadable... T>
std::size_t total_bytes(const T&... assets) {
    std::size_t result = 0;
    [[maybe_unused]] const auto add_checked = [&result](std::size_t next) {
        if (next > std::numeric_limits<std::size_t>::max() - result)
            throw std::overflow_error("batch size overflow");
        result += next;
    };

    (add_checked(asset_bytes(assets)), ...);
    return result;
}
```

The `(expression, ...)` form is a **unary right fold over the comma operator**. For the built-in comma behavior selected here, evaluations proceed in the specified left-to-right order: determine the current asset size, validate its addition against the remaining numeric range, then add it. With three assets of 12, 48 and 15 bytes, the intermediate totals are 12, 60 and 75 bytes. That trace should be reproducible without running the program. The result is not 75 kilobytes and not an allocated GPU buffer size. It is exactly 75 source payload bytes under the demonstrated type-specific formulas.

The empty pack matters. A unary comma fold is permitted with no operands and has `void()` as its identity, so the function can return the initialized zero. In our initial strict GCC test, however, the local checked-addition lambda was then defined but unused, triggering a warning elevated to a compilation error by `-Werror`. The final lab explicitly marks it `[[maybe_unused]]`, recognizing that the empty-pack specialization does not need a callable. This is a useful example of real template instantiation behavior: source that works when instantiated with three arguments can fail under a different pack size or compiler diagnostics. The zero-argument path is part of the contract and therefore must be compiled and tested.

Fold expressions are not universally evaluated left-to-right. Their grouping and operand evaluation follow the particular operator's rules; for many operators, grouping does not imply the execution order of arbitrary side effects. We intentionally use a built-in comma fold because sequencing is an important requirement here. A production batch planner might instead return a checked result type, distinguish overflow from unsupported asset type, and enforce hardware-specific alignment separately. A fold does not remove any of those requirements.

### A generic function still needs a boundary describing which types qualify

The traits model is useful only if unsupported types fail early and understandably. Without a public constraint, a caller can instantiate `asset_bytes(UnknownAsset{})` and receive an error somewhere inside a dependent function body. We would prefer the interface to state what a supported asset must provide.

Historically, C++ libraries often used **SFINAE**—*Substitution Failure Is Not An Error*—for this purpose. The phrase has a precise scope. During template argument substitution in certain immediate contexts, an invalid substitution can remove a candidate from consideration rather than producing an immediate hard error. It does not mean arbitrary invalid code in a template body is harmless. It also does not make a runtime bad input acceptable.

Here is the lab's representative detection trait:

```cpp
template <class T, class = void>
struct HasByteSize : std::false_type {};

template <class T>
struct HasByteSize<T, std::void_t<decltype(
    AssetTraits<T>::byte_size(std::declval<const T&>())
)>> : std::true_type {};
```

The primary definition reports false. The partial specialization is considered when the expression inside `decltype` is valid during substitution. `std::declval<const T&>()` supplies a type-level expression in an unevaluated context; it does not construct a real `T`. `decltype` asks about an expression's type, not its runtime result. If the expression is invalid for `UnknownAsset`, the specialization is discarded and the primary false trait remains. If it is well-formed for `Mesh` or `Texture<4>`, the specialization reports true.

The important limitation is that this trait checks whether a particular **expression is well-formed**. It does not check whether a size calculation is numerically correct or whether a device can accept the resulting buffer. The trait does not execute the function body, so a function that always throws could still satisfy the detection rule. Nor does this form necessarily produce the clearest diagnostics for a new contributor. C++20 **concepts** and **requires-expressions** make the public requirement far easier to read.

### Write the contract in the type system at the point where it is needed

Our asset protocol requires two things: a readable type-specific label and a byte-size expression producing `std::size_t`. We can state exactly that as a **concept**, a named compile-time predicate:

```cpp
template <class T>
concept Uploadable = requires(const T& asset) {
    { AssetTraits<T>::label } -> std::convertible_to<std::string_view>;
    { AssetTraits<T>::byte_size(asset) } -> std::same_as<std::size_t>;
};
```

A **requires-expression** checks expressions and type relationships for well-formedness under its parameter declarations. The braces around a compound requirement permit a return-type requirement introduced by `->`. The labels `std::convertible_to<std::string_view>` and `std::same_as<std::size_t>` are themselves standard-library concepts. The latter expresses an exact type requirement for the declared byte-size expression; a size function returning `int` is not accepted as an exact match merely because an integer can be converted to `std::size_t`. The name `Uploadable` means "has the static interface required by this laboratory", not "this object is guaranteed safe to upload".

Critically, the `requires` expression examines the expression's validity **without executing a real upload or checking data-dependent values**. A `Texture<4>` with an impossibly large texel count can still satisfy the concept. The call to `byte_size` must later reject arithmetic overflow at runtime, which our lab explicitly tests. Concept satisfaction is a **compile-time structural contract**; semantic truth and runtime preconditions require further evidence.

The laboratory includes:

```cpp
static_assert(Uploadable<Mesh>);
static_assert(Uploadable<Texture<3>>);
static_assert(Uploadable<Texture<4>>);
static_assert(!Uploadable<UnknownAsset>);
```

These check that the declarations and trait specializations match our intended boundary. The template definition also constrains the store itself:

```cpp
template <Uploadable T, std::size_t Capacity>
class AssetDepot {
    // Only types meeting Uploadable can form usable specializations here.
};
```

The concise `template <Uploadable T>` form abbreviates a constraint on `T`; it does not transform the chosen asset into a new base class or inject a virtual interface. Types remain concrete. The API can then add a more specific **requires-clause** for an operation that constructs them:

```cpp
template <class... Args>
    requires std::constructible_from<T, Args&&...>
AssetId emplace(Args&&... args);
```

This says: not every supported asset is constructible from every possible argument list. A mesh might accept a vector of indices; it should not silently accept an unrelated integer and guess what it means. Constraint checking can reject the invalid call during overload resolution. The lab goes further: it defines a `CanEmplace` concept that tests the actual expression against a store type, proving both vector-lvalue and vector-rvalue calls are available and an arbitrary integer call is not.

There are subtle boundaries here. A requires-clause can be written after the template parameter list or after a function declarator; concepts can also constrain abbreviated function templates. The compiler normalizes constraints to compare them for overload selection, and semantically similar Boolean expressions are not automatically interchangeable for that comparison. Reusing a **named concept** creates a stable requirement vocabulary and often makes constrained overload ordering more predictable than independently restating complicated conditions.

### Why two overloads can share a name without creating a runtime type switch

The editor wants a diagnostic route function that produces a useful label for recognized assets and a fallback for anything else. One unconstrained function template can say "unsupported"; a second, otherwise similarly shaped function template can require `Uploadable` and use the trait:

```cpp
template <class T>
constexpr std::string_view route(const T&) {
    return "unsupported";
}

template <Uploadable T>
constexpr std::string_view route(const T&) {
    return AssetTraits<T>::label;
}
```

Both overloads are candidates for a `Texture<4>` value, but the constrained overload is more specialized under the relevant rules, so it produces `"texture"`. For `UnknownAsset`, the constrained overload's requirement is not satisfied, leaving the fallback `"unsupported"`. This is resolved at compile time for the statically known argument type; it is not selecting a derived object's virtual method at runtime.

**Constrained overload resolution** uses ordinary viability and conversion rules together with template partial ordering and constraint subsumption. Constraint subsumption is not a general theorem prover. In particular, two independently written expressions that happen to mean the same thing to a mathematician may not count as the same atomic constraint under the language's identity rules. Writing `requires(N > 0)` and `requires(N >= 1)` does not establish a universal compiler-recognized ordering relationship simply because integers make the statements logically related. The precise normalization and identity rules matter. Named concepts help organize reusable constraints, but one must still test potentially overlapping overload sets for ambiguity.

The important design insight is that **specialization chooses the type-specific asset rule**, while **a constrained overload chooses which static interface may be called**. The public store API therefore has a predictable and explainable set of permitted operations. It does not accept an arbitrary pointer and hope a cast will work at runtime.

### Publication still needs a correctness proof after introducing templates

A generic container can be perfectly well-typed and still violate the exception guarantee established in Phase 2. We must therefore prove the new store's publication logic with the same care as its compile-time API. The reference store retains `std::array<std::optional<T>, Capacity>` and an integer count of published entries. Its insertion is:

```cpp
if (count_ == Capacity)
    throw std::length_error("asset depot full");

slots_[count_].emplace(std::forward<Args>(args)...);
return AssetId{count_++};
```

This sequence has a deliberate order. The capacity check happens before construction. The next slot is unoccupied by the store invariant. `optional<T>::emplace` constructs the candidate object in the currently unpublished slot; if the constructor throws, no new completed asset is published and `count_` remains unchanged. Only after construction returns successfully does `count_++` add the slot to the published prefix. With this append-only design, the earlier slots remain unaffected. There is no vector reallocation in this particular storage layout, so the strong-state argument differs from Phase 2's `std::vector` proof.

Let `c` be the number of published entries before the call and `C` the static capacity. We maintain the invariant:

\[
0\le c\le C,\qquad
\text{slots }[0,c)\text{ are engaged},\qquad
\text{slots }[c,C)\text{ are disengaged}.
\]

If `c=C`, the function rejects the insertion and changes nothing. If `c<C` and the proposed construction throws, the next optional remains disengaged and the published count stays `c`. If construction succeeds, the new slot is engaged, and incrementing the count makes the published interval `[0,c+1)`. This is a short, inspectable correctness proof that depends on the container's real semantics, not merely the word "generic".

The lab checks a `CheckedShader` whose constructor throws for a nonpositive version number. The first shader (version 3) is published in slot zero. The attempt to emplace version −1 throws; the store still reports one published asset, and the first shader remains valid. Emplacing version 4 then successfully reuses the *still-unpublished* slot one. The mesh store also tests capacity overflow: once its two slots are full, an extra insertion throws before touching existing assets.

This code does **not** implement removal or recycled slots. An asset ID can be used only while its depot is alive and while the append-only invariant holds. There is no thread synchronization: concurrent publication or access is not part of the proven contract. And `optional<T>` does not give external GPU resources a completion fence. It constructs and destroys C++ values under ordinary language rules; a resource-owning value stored inside it still needs a correct backend release protocol.

### The real performance question is what gets instantiated, stored and moved

Generic code is often described as a "zero-cost abstraction". That phrase is meaningful only when we specify *relative to which baseline, under which workload, and which cost*. A function template instantiated for `Mesh` and `Texture<4>` can produce distinct specialized functions. The compiler may inline them, use the known channel count to simplify multiplication or combine equivalent machine code. It may also produce more binary code, longer compilation times and different instruction-cache behavior. The C++ language does not promise that a particular template call becomes one machine instruction or that a distinct instantiation always occupies distinct final machine-code bytes.

The fixed `AssetDepot<T,Capacity>` stores `Capacity` optional slots inline. That means its memory cost grows with both the number of slots and the size/alignment of `T` plus any representation overhead needed by the optional state. The exact size is implementation-dependent; writing `Capacity*sizeof(T)` as an equality would be misleading. Heap allocations inside `T`, such as a mesh's vector storage, remain separate from the fixed depot slots. A reference obtained from a slot can remain at a stable address during subsequent successful appends, but it still dangles if the depot itself dies or moves in a way that invalidates those references. Static capacity does not make lifetime management disappear.

A production comparison would create equally capable versions using fixed optional slots, a growing vector, and a paged/generational registry, then measure construction success/failure, byte footprint, allocations, iteration locality, compile time and hot-loop behavior. A runtime polymorphic asset wrapper may be necessary when a collection must contain *unrelated asset types together under one dynamic interface*. A `variant` may be appropriate for a closed set of heterogeneous values. A class template is appropriate when callers statically know the stored `T`. These mechanisms solve different problems; selecting them by slogan would undo the work of understanding their contracts.

The first meaningful research-style experiment from this chapter would be to prepare thousands of mesh-like and texture-like records under different size distributions, inject constructors that fail at controlled points, measure publication invariants and allocations, and compare the generic implementation with a hand-written concrete baseline. Another would measure whether compile-time channel counts result in improved throughput after compiler optimization and whether the additional specializations enlarge the binary enough to affect instruction-cache behavior. Such outcomes must be measured, not invented from template syntax.

### One complete trace and a reconstruction challenge

The tested program constructs a two-slot mesh depot. It starts with indices `{3,7,9}` and inserts them first as an lvalue and then as a moved rvalue. The first mesh records that it copied; the second records that it received the move path. Both contain the expected index count. A third insertion is rejected because capacity is exhausted, and the published mesh count remains two. A shader depot accepts version 3, rejects version −1 without publishing, then accepts version 4 into its next slot. Next the program constructs a four-channel texture of 12 texels and a three-channel texture of 5 texels. Their byte sizes are 48 and 15 respectively; the three-index mesh contributes 12. The checked heterogeneous batch function produces **75 bytes**, and its empty-pack case produces zero. `route` chooses `"mesh"` and `"texture"` for supported types and `"unsupported"` for an unknown type. Finally, a deliberately enormous texture count triggers the overflow check before the multiplication could wrap.

The entire trace is covered by assertions that stay active in a release-style test build, with compile-time assertions guarding unsupported types and constructor argument patterns. The reference program has been compiled with GCC 14.2 and address/undefined-behavior sanitizers, Clang 17 with warnings-as-errors, and GCC with `-O2 -DNDEBUG`. Each run reported `generic_asset_pipeline_lab: all checks passed`. The source committed to this repository was compared byte-for-byte against the compiled source using Git blob SHA `e673da4c89e50114b3e49c5b1b32dda7bf019ec8`. This is concrete evidence for the standalone C++20 model, **not** a source audit of Kairo, a real GPU pipeline, a multithreaded asset registry, or exhaustive fault injection.

Build it yourself from the repository root:

```sh
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  tools/labs/generic_asset_pipeline_lab.cpp -o /tmp/generic_asset_pipeline_lab
/tmp/generic_asset_pipeline_lab
```

To reconstruct it from memory, first explain why copying three independent stores is a maintenance problem but replacing them with raw `void*` is a type and lifetime problem. Build a function template that deduces an asset type, then a class template whose non-type capacity parameter determines the slot array. Write a full trait specialization for `Mesh` and a partial specialization for `Texture<Channels>`. From first principles, derive the four reference-collapsing cases; show why a named vector chooses the copy constructor and a forwarded rvalue chooses the transfer path. Implement a variadic batch-size function with overflow checks and an empty-pack result, then explain the fold's sequencing. Replace a SFINAE detection trait with a named concept and a requires-expression, constrain store construction and prove the fallback/constrained overload selection. Finally, trace a throwing constructor through the unoccupied slot to show exactly why previously published values remain untouched.

The original source contract calls this **M013, Parts 201–216**. The chapter is *authored reconstruction*, not a received original M013 transcript; the source ledger maps each part to its place in the story and its specific evidence. [[Supplementary/Foundations#M013 — Parts 201–216: generic asset preparation with constrained APIs|M013 coverage map]] · [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#One asset pipeline, many types — M013 retrieval|reconstruction board]]. We have not inspected current `KairoMath/Matrix.cppm` or `KairoECS/SparseSet.cppm` revisions, which are original curriculum audit targets rather than proven facts about external repositories.

## A texture tile that chooses its work before the frame begins

The generic asset pipeline is now able to prepare meshes, textures, and shader records without copying the entire storage implementation for each type. An `AssetTraits` specialization describes the type-specific facts; a constrained `AssetDepot` constructs its selected `T` only after the argument types are validated; and a published prefix remains unchanged when the next constructor fails. That gives us a good type system and a readable ownership contract, but a renderer faces an additional question that occurs thousands or millions of times: **which decisions can be settled while we build the program, and which decisions must still happen for every resource or frame?**

Consider one practical operation. A texture arrives in a known pixel format. We need to determine its channel count, check the supplied dimensions, estimate the number of bytes, decide whether the tile fits a budget, perhaps compute a small spatial-addressing key, and record a work plan. If the format and the validation policy are compile-time choices, repeatedly asking what the format is during every pixel operation seems unnecessary. But if the width, height, source bytes, and runtime memory budget come from the asset or user, treating those as predetermined compile-time constants would be a mistake.

The solution is not “move everything into templates.” We must split the **stable configuration known to the compiler** from **the values that arrive while the program runs**, while preserving correct checks at their respective boundaries. That will naturally bring us to type traits, compile-time functions, policy types, static dispatch, lookup tables, and memory layout. The laboratory for this chapter is [[tools/labs/compile_time_asset_policy_lab.cpp|a standalone C++20 tile-planning program]]. It plans CPU-side work and validates an optional source payload; it does not pretend that producing a work plan has allocated GPU memory or submitted a Metal/Vulkan command.

### The pixel format is not an integer whose meaning we must rediscover

Suppose the tile is an 8-by-2 image with four 8-bit channels per pixel. It has \(8\times 2=16\) pixels. Each pixel contributes four source bytes, so its uncompressed, tightly packed payload has \(16\times4=64\) bytes. For an RGB image with four rows and four columns, the source payload has \(4\times4\times3=48\) bytes. These equations arise from the representation we have chosen: width and height count pixels, while `channels` counts 8-bit source components per pixel. They do **not** automatically give the amount of device memory after alignment, row-pitch requirements, mipmaps, block compression, or backend-specific allocation overhead.

Our asset type already knows its format. We therefore define a finite domain:

```cpp
enum class PixelFormat { R8, RGB8, RGBA8 };

template <PixelFormat Format>
struct PixelTraits; // intentionally no definition for arbitrary values

template <>
struct PixelTraits<PixelFormat::R8>
    : std::integral_constant<std::size_t, 1> {};

template <>
struct PixelTraits<PixelFormat::RGB8>
    : std::integral_constant<std::size_t, 3> {};

template <>
struct PixelTraits<PixelFormat::RGBA8>
    : std::integral_constant<std::size_t, 4> {};
```

We already met full and partial specializations when the mesh and texture types required different size rules. The new idea is that the pixel-format trait **inherits from a type that represents a compile-time constant**. `std::integral_constant<T,V>` is a class template whose `value` is the constant `V` of type `T`. It is often used to convert a compile-time fact into an object-like type that generic machinery can inspect. `std::true_type` and `std::false_type` are familiar instances of this pattern with Boolean values. For example, `PixelTraits<PixelFormat::RGBA8>::value` is the compile-time integer four.

Why make a value available as a type rather than simply store four in a field? Because the compiler can choose definitions and overloads using the type. It can evaluate bounds and compile-time assertions before execution, and it can form an operation specialized to the exact selected format. **Type traits** are the general category of compile-time questions about types or their associated properties: is a policy class empty? Is this resource wrapper nothrow movable? Is this asset type trivially copyable? Do two types denote the same type? The standard library answers many such questions through traits such as `std::is_empty_v<T>`, `std::is_trivially_copyable_v<T>`, and `std::is_same_v<T,U>`. The suffix `_v` is the convenient value form, generally based on the corresponding class template's `::value`.

These traits are facts about the **specified C++ type property**, not magical correctness checks. `std::is_empty_v<Policy>` cannot prove the policy is thread-safe or that it leaves validation unchanged. `std::is_trivially_copyable_v<T>` cannot prove that `T` has a stable wire format, as the preceding chapter demonstrated. Our format traits also do not validate a corrupt external image: they describe the chosen *internal model*. We must still verify file metadata, decode external formats, and reject unsupported values before forming an internal typed description.

The lab uses compile-time assertions to confirm that the RGB trait supplies three and that the RGBA trait derives from `std::integral_constant<std::size_t,4>`. This is a small but important transition: an external pixel count is data, while the chosen internal format's channel count is part of the program's type-level contract.

### The renderer asks whether one branch can disappear completely

Now consider a tile-planning function which wants to mark whether a format includes an alpha component. It could compare a runtime enum on every call. But the function already has `Format` as a template parameter. We can use:

```cpp
bool alpha = false;

if constexpr (Format == PixelFormat::RGBA8) {
    alpha = true;
}
```

The expression in `if constexpr` is a compile-time condition. For an instantiation with `Format == PixelFormat::RGBA8`, the selected branch is kept; for a different format the discarded statement is not instantiated in the same way as ordinary runtime control flow. This is especially useful when branches contain type-dependent operations that are well-formed only for certain types. A regular `if` requires both branches to be valid C++ for the instantiated function, even if a runtime condition always happens to be false. `if constexpr` can remove the need to compile a type-dependent discarded branch once the condition is fixed.

There are boundaries. `if constexpr` is not a preprocessor switch that arbitrarily hides invalid non-dependent code. Outside templates, the compiler still checks many forms of invalid code even in a branch that would be discarded; inside a template, names and expressions have dependent versus non-dependent validity rules. The selected branch can still perform runtime work. And an ordinary compiler may optimize a constant regular `if` to the same machine code anyway. We choose `if constexpr` primarily to **express an intended compile-time semantic distinction**, not to claim a guaranteed speedup without examining generated code.

In the tested program, the RGBA specialization reports `alpha=true` and the RGB specialization reports `alpha=false`. The tile's width and height are still runtime arguments. They retain their own checks, even though the channel count is fixed. A compile-time choice about **how** an operation works does not make the operation's **input values** valid.

### Some calculations can be used at compile time without being forced there

Before allocating or planning, the renderer multiplies dimensions. A naive `width * height * channels` may overflow `std::size_t` and wrap modulo its unsigned range. That is dangerous for allocation and bounds decisions. We need a function which is useful both when values are known at compile time and when they arrive from a file at runtime:

```cpp
constexpr std::size_t checked_product(std::size_t x, std::size_t y) {
    if (y != 0 && x > std::numeric_limits<std::size_t>::max() / y)
        throw std::overflow_error("byte-count multiplication overflow");
    return x * y;
}

static_assert(checked_product(8, 4) == 32);
```

The word `constexpr` means this function is **eligible** for evaluation in a constant-expression context when the necessary conditions hold. It does not say every invocation executes during compilation. In the static assertion, `checked_product(8,4)` is evaluated as a constant expression. When we later call `checked_product(runtime_width,runtime_height)`, the same source function may execute at runtime. A compiler may also optimize a runtime call using ordinary constant propagation where it has sufficient information; that optimization is distinct from the language-level requirement to produce a constant expression.

Why is the overflow check written as a division? Multiplying first would already lose the information we needed to detect overflow. For nonnegative unsigned numbers \(x,y\), a product fits in a maximum representable value \(M\) exactly when \(x\le M/y\) for \(y\ne0\). We therefore test the inequality before multiplication. The `y == 0` case needs separate handling to avoid division by zero; multiplying by zero is safe and yields zero. In the tile pipeline, we apply this function once to `width * height` and again to `pixels * channels`. This establishes a correct *count arithmetic* invariant, although it does not establish enough available memory or any GPU-specific alignment guarantee.

At runtime, the laboratory asks for a tile with width `SIZE_MAX` and height two. The first multiplication check throws before computing an overflowing product. The policy and recorder are not invoked after that failure, so the rejected request does not appear as a planned tile. In a compile-time constant-evaluation context, a call that attempts to evaluate the throwing path cannot produce the required constant expression; the program must reject its use as a compile-time constant. This is not an excuse to rely on exceptions in a real-time hot loop: the public engine may instead return a typed error, as Phase 2 developed. The function illustrates the distinction between the *arithmetic rule* and the *chosen error-delivery policy*.

### Configuration that must be known before the program executes

The renderer may choose a supported tile shape at build time, such as an 8-by-8 block. That choice could determine constant buffers, threadgroup dimensions or a compile-time lookup table in a backend-specific implementation. A normal `constexpr` function may be called at runtime, which is useful for arithmetic but not quite the contract we want for an immutable build configuration. C++20 provides `consteval`:

```cpp
struct TileDefinition {
    std::size_t width, height;
};

consteval TileDefinition make_tile_definition(std::size_t width,
                                               std::size_t height) {
    if (width == 0 || height == 0 || width > 64 || height > 64)
        throw "tile dimensions must be in [1, 64]";
    return {width, height};
}

inline constexpr TileDefinition kDefaultTile =
    make_tile_definition(8, 8);
```

A `consteval` function is an **immediate function**. Its potentially evaluated calls must satisfy the language's immediate-invocation rules and produce a constant expression where required. The invalid-dimension path has no acceptable constant result. A call such as `make_tile_definition(runtime_width,runtime_height)` in an ordinary runtime context is not simply another runtime validation operation; it is ill-formed if the argument values cannot satisfy the constant-evaluation requirement. That difference is the point of writing `consteval`.

The returned `TileDefinition` is stored in an `inline constexpr` variable, whose value is available as a constant expression. The `inline` specifier here supports coherent definitions in headers across translation units according to the language's rules; it does **not** mean the variable must be physically duplicated in every executable object or that the compiler has to perform a particular inlining optimization. The lab checks that the chosen tile holds 64 pixels. A caller-provided tile dimension still needs **runtime** validation, as shown in `TilePlanner::prepare`; it would be wrong to pretend that validating an 8-by-8 default validates every image loaded by the user.

Three terms are now distinguishable. `constexpr` enables constant evaluation of functions and designates qualifying variables as constant-expression values. `consteval` requires immediate compile-time evaluation for the function's relevant calls. `constinit`, which we will need for a global diagnostic counter, guarantees **static or thread storage duration initialization is constant-initialization**; it does not make the resulting variable immutable. Those are different contracts, even though their names look similar.

### Why precompute a spatial-address lookup instead of rebuilding it?

The same engine that loads textures may divide images into small tiles and assign compact keys to neighboring positions. One common ordering scheme is **Morton order**, also called Z-order, which interleaves the binary digits of two coordinates. It is useful because coordinates that are near one another often receive nearby keys, but it does not guarantee perfect spatial locality or replace a complete spatial index.

Take a four-bit coordinate \(x\). Write its bits as \(x_3x_2x_1x_0\). To make room for a second coordinate, place each bit at an even position:

\[
\operatorname{spread}(x)
=\sum_{i=0}^{3} \left(\left\lfloor\frac{x}{2^i}\right\rfloor\bmod 2\right)2^{2i}.
\]

The division and remainder here simply ask for bit \(i\); in C++ we use a right shift and bitwise AND. For \(x=3\), binary `0011`, the low two bits are set, so spreading produces `00000101`, decimal five. For \(y=5\), binary `0101`, spreading produces `00010001`, decimal seventeen. Shift the spread \(y\) bits left by one and combine:

\[
\operatorname{morton}(3,5)
=\operatorname{spread}(3)\;|\;
\big(\operatorname{spread}(5)\ll 1\big)
=5\;|\;(17\ll1)
=5\;|\;34
=39.
\]

The bitwise OR is correct here because one spread result occupies even bit positions and the other, after shifting, occupies odd bit positions; there is no overlapping set bit to carry. If the coordinates are outside the range zero through fifteen, the four-bit function cannot represent them and must reject them or use a wider scheme. This small example is **one four-bit-per-axis tile key**, not a complete texture layout for arbitrary image dimensions.

We can write the bit-spreading calculation once as a `constexpr` function and generate all sixteen possible values at compile time:

```cpp
constexpr std::uint8_t spread4(std::uint8_t n) noexcept {
    std::uint8_t result = 0;
    for (unsigned bit = 0; bit != 4; ++bit) {
        result |= static_cast<std::uint8_t>(
            ((n >> bit) & 1u) << (2u * bit)
        );
    }
    return result;
}

consteval std::array<std::uint8_t, 16> make_morton_table() {
    std::array<std::uint8_t, 16> table{};
    for (std::size_t n = 0; n < table.size(); ++n)
        table[n] = spread4(static_cast<std::uint8_t>(n));
    return table;
}

inline constexpr auto kSpread4 = make_morton_table();
static_assert(kSpread4[3] == 5 && kSpread4[5] == 17);
```

This is a **compile-time lookup table**. The expensive-to-explain transformation is fully specified, and each possible four-bit input has an entry known before runtime. The actual function `morton4(x,y)` validates the two coordinates, reads their table entries and interleaves them. The lab checks the derived result `morton4(3,5)==39` both at compile time and during execution.

A table is not automatically faster than direct bit operations. On some processors, a handful of shifts and masks can be cheaper than additional memory loads; for other workloads a table may be beneficial, especially if it remains in cache. A wider table increases data footprint, which can harm locality. On the GPU, wavefront execution, cache layout and address calculation can change the answer again. The purpose here is to show *when compile-time precomputation becomes available* and how to validate its meaning. A fair performance experiment must compare these algorithms on the same machine and workload, not assume the presence of `constexpr` guarantees a speedup.

### A global counter illustrates a different constant-initialization promise

During testing, we want to count plans that reached the recorder. A global variable can have **static storage duration**, and it may be accessed long after the particular planner object was constructed. But we do not want its initialization to depend on dynamic initialization performed by another translation unit. We declare:

```cpp
constinit std::uint32_t g_plans_recorded = 0;
```

This variable is initialized in the static-initialization phase under the `constinit` rules. The compiler rejects an initializer that cannot meet the required constant-initialization contract. Unlike `constexpr`, `constinit` does not require the variable to be const. In the single-threaded lab we increment it after successful recorder calls and verify the final value two.

This does **not** make a global mutable counter good architecture. It is neither automatically atomic nor a synchronization primitive. If multiple worker threads incremented it without synchronization, they could create a data race. A production diagnostic counter would likely use an appropriate atomic or aggregate per-thread counts and combine them under a defined memory ordering and reporting policy. `constinit` solves an **initialization-order concern**, not runtime concurrency, visibility or ownership. A separate global with non-trivial dynamic initialization can still impose dependencies, and initializing one object early does not establish a lifetime relationship with device work.

### One validation rule must vary without making every call virtual

Our tile planner always needs to check arithmetic overflow, positive dimensions and, when verifying a source buffer, its byte length. But different contexts may apply different **budget policies**. An editor's import queue might cap one tile at a specified number of bytes. A metadata-only discovery pass may have no additional application-specific byte budget, while still rejecting impossible dimensions and overflowing arithmetic.

We represent those policies as ordinary types:

```cpp
struct NoBudgetPolicy {
    void validate(std::size_t) const noexcept {}
};

struct ByteBudgetPolicy {
    std::size_t maximum;

    void validate(std::size_t count) const {
        if (count > maximum)
            throw std::length_error("tile exceeds byte budget");
    }
};
```

A policy is a **choice about how an operation is performed**, separated from the larger operation that uses it. The type `NoBudgetPolicy` carries no stored state, whereas `ByteBudgetPolicy` stores a numeric limit. The phrase "no budget" means no *additional application cap*; neither policy bypasses width/height overflow or, in verified-payload mode, the actual buffer-length check. Choosing one policy through a template parameter means that the compiler knows which `validate` function the planner calls. If we instead needed to change policy at runtime using a common interface, a virtual policy or type-erased function might be more appropriate. This is precisely the distinction between policy-based static choice and dynamic extensibility.

The planner is parameterized by both format and policy:

```cpp
template <PixelFormat Format, class BudgetPolicy, class Recorder>
class TilePlanner {
    [[no_unique_address]] BudgetPolicy budget_;
    Recorder recorder_;

public:
    TilePlanner(BudgetPolicy budget, Recorder recorder)
        : budget_(std::move(budget)), recorder_(std::move(recorder)) {}

    // prepare(width, height, payload, mode) is implemented in the lab.
};
```

The policy instance's *type* is fixed by the specialization, but its stored state, such as a 64-byte budget, can still be supplied at runtime. That distinction is important: choosing `ByteBudgetPolicy` at compile time does not mean the number 64 must be a template argument. We can change budgets between planner objects without compiling a different type for every numeric value.

For the RGBA example, width eight and height two require \(8\times2\times4=64\) bytes. With a 64-byte budget, the plan is accepted and recorded. An 8-by-3 tile would require 96 bytes, so the same planner throws before recording it. A different 4-by-4 tile also requires 64 bytes, but when the verified-payload mode is used with an empty buffer, it fails the length check. Both failure paths leave the recorder's count at one. The error source and the publish boundary remain observable; policy selection does not hide them.

### The recorder is known statically; explain the CRTP rather than mystifying it

The planner must report successful work plans. In Phase 1 we used a virtual `Backend` because a runtime-selected implementation had to satisfy one interface. The tile planner can make a different choice: it can know its recorder implementation as a template argument. The **Curiously Recurring Template Pattern**, or **CRTP**, is one way to express this static relationship:

```cpp
template <class Derived>
struct RecorderBase {
    void record(const TilePlan& plan) {
        static_cast<Derived&>(*this).record_impl(plan);
    }
};

struct CountingRecorder : RecorderBase<CountingRecorder> {
    std::size_t calls = 0;
    std::size_t total_bytes = 0;

    void record_impl(const TilePlan& plan) {
        ++calls;
        total_bytes += plan.bytes;
    }
};
```

The derived class declares itself as the base template's parameter. The base function can therefore convert its `*this` to the named derived type and call `record_impl`. The compiler resolves the concrete call through known types rather than through a virtual function table. This is *static polymorphism*—different compile-time types expose a common operation without requiring an ordinary runtime virtual interface.

The cast is not an arbitrary trick that makes any base object a derived object. The pattern relies on the real object being a correctly formed `Derived` whose `RecorderBase<Derived>` base subobject is being used. Constructing an unrelated base object and invoking the derived cast would not establish a valid derived object. The derived operation must also exist and be accessible under the chosen design. Concepts or direct constrained calls can communicate many of those requirements more explicitly, especially in modern C++; CRTP is a tool, not compulsory boilerplate.

The lab creates a `CountingRecorder`, submits one accepted 64-byte plan, and verifies both `calls==1` and `total_bytes==64`. Failed plans never reach `record`. The plan itself is a small value containing dimensions, byte count, one illustrative Morton key and whether the format includes alpha. These fields describe **planned work**; they are not proof of completed GPU transfer.

Why not always use CRTP? A `TilePlanner<Format,Policy,CountingRecorder>` is a different type from a planner instantiated with another recorder. If the editor must store arbitrary recorder implementations in one container and select them at runtime, it needs another abstraction, such as an erased callback or virtual interface. CRTP can enable inlining, but whether it does so is an optimization question; source-level absence of `virtual` is not a measured throughput comparison. It may also increase code size through multiple instantiations. We must select the dispatch mechanism according to whether the variation occurs at compilation or execution.

### The same operation can be verified or merely described—without a Boolean flag

There is another source of variation: should `prepare` check the actual byte buffer, or should it produce **only a metadata plan** when no buffer is available? A Boolean `verify_payload` would work, but it is easy for a caller to confuse a plan with a verified upload or to leave a condition buried in a long method. Since the caller knows which operation it wants at the call site, we can represent the decision using distinct empty **tag types**:

```cpp
struct VerifyPayloadTag {};
struct MetadataOnlyTag {};

void check_payload(std::span<const std::byte> bytes,
                   std::size_t expected, VerifyPayloadTag) {
    if (bytes.size() != expected)
        throw std::invalid_argument("payload size mismatch");
}

void check_payload(std::span<const std::byte>,
                   std::size_t, MetadataOnlyTag) noexcept {
    // Deliberately produce a plan only: no verified-buffer claim.
}
```

This is **tag dispatch**. The tag argument is an object of a type selected by the caller. Ordinary function overload resolution chooses the matching function at compile time, because `VerifyPayloadTag` and `MetadataOnlyTag` are distinct argument types. The compiler does not need a runtime string or integer to choose between the operations. An empty tag normally contains no application data; its *type* is what carries the decision.

The semantics matter more than the dispatch technique. A request with an empty payload must be rejected in verified mode if the expected size is nonzero. In metadata-only mode, the same expected size may be calculated and recorded, but the planner **must not claim the payload exists, is decoded, or is ready for a GPU consumer**. The lab tests this exact difference. A 4-by-4 RGB tile produces a 48-byte manifest without source data; the result is explicitly a plan, not an upload. Tags create a readable distinction, but a production API might go further and return different types for `VerifiedTile` and `TileManifest` so that downstream code cannot confuse them. The current lab does not implement that stronger distinction.

Tag dispatch and `if constexpr` are related but not identical. Tags select among overloads using distinct parameter types; `if constexpr` selects an instantiation branch within one function template. Policies carry a larger strategy and may store runtime state. CRTP supplies a compile-time-known base/derived call shape. These mechanisms can cooperate, but using all of them in every class would make ordinary code needlessly elaborate. Here each appears only because a specific decision—payload readiness, format, budget, or recorder implementation—has a distinct useful contract.

### If a policy has no data, why can its ordinary member increase object size?

We now have a no-budget policy object that has no non-static data members. It exists as a real C++ type, but adding it as an ordinary member to every planner may seem wasteful. The reason requires the earlier memory-layout model: C++ objects and subobjects have identity and alignment rules, and a normal empty class type still occupies at least one byte when instantiated as a complete object. Its size and the padding introduced around it can affect the containing class.

Consider three structurally similar holders:

```cpp
struct PlainPolicyHolder {
    NoBudgetPolicy policy;
    std::size_t counter;
};

struct EboPolicyHolder : private NoBudgetPolicy {
    std::size_t counter;
};

struct CompactPolicyHolder {
    [[no_unique_address]] NoBudgetPolicy policy;
    std::size_t counter;
};
```

The first has an ordinary empty member, so the implementation must represent its identity subject to the language rules; alignment of the following `std::size_t` can introduce padding. The second uses inheritance from an empty base. **Empty-base optimization (EBO)** allows the implementation to avoid additional storage for certain empty base subobjects, while satisfying the relevant address/identity constraints. The third uses C++20's `[[no_unique_address]]` attribute. It makes the data member a *potentially overlapping subobject*, allowing an implementation to overlap its storage with other eligible members or padding where the language permits.

On the tested Linux GCC/Clang setup, the sizes printed are:

| Holder | Observed `sizeof` |
|---|---:|
| `PlainPolicyHolder` | 16 bytes |
| `EboPolicyHolder` | 8 bytes |
| `CompactPolicyHolder` | 8 bytes |

These are **local observations**, not universal results. Different pointer widths, data models, alignment rules, compilers and ABI policies can produce different layouts. The language's constraints about which subobjects may share addresses are subtler than "all empty objects take zero bytes." For example, potentially overlapping subobjects of the same type may still have address-distinctness obligations, and the location of the first member and base classes can limit EBO arrangements. For nonempty members marked `[[no_unique_address]]`, eligible tail padding may also be reused in some layouts; that does not allow us to invent overlapping live object representations or an invalid serialization scheme.

There is a particularly important portability detail: the standard spelling `[[no_unique_address]]` is supported in modern GCC and Clang as tested here, but Microsoft Visual C++ has historically not honored that exact attribute for layout in C++20 mode, providing `[[msvc::no_unique_address]]` as an alternative extension. A cross-platform library cannot assume one attribute has the same size effect on every target. Confirm the actual toolchain behavior and relevant ABI compatibility before using object-size changes as an interface promise. See the cited portability reference in the verification ledger.

Why prefer a `[[no_unique_address]]` member in this planner rather than deriving privately from the policy? Because composition more directly communicates "this planner *has a validation policy*" and avoids some inheritance constraints, while still giving a conforming compiler freedom to avoid unnecessary policy storage. That is a design preference for this example, not a claim that all inheritance is harmful. An empty policy is still meaningful through its **type and behavior** even if a particular object representation adds no size.

The key distinction is **source-level capability versus observable layout**. We can prove that `NoBudgetPolicy` is empty with a type trait. We can make its member eligible for overlap with `[[no_unique_address]]`. We cannot prove that the compiled planner occupies a specific number of bytes until the relevant compiler, ABI and member types are fixed and measured. Nor should a raw class-layout measurement be confused with GPU buffer packing or an on-disk format.

### Static choice and runtime choice have different ownership and performance consequences

Return to the recorder. With CRTP, the concrete `CountingRecorder` is part of the planner's type, so the call target is known while compiling that specialization. We might instead pass a runtime-selected callable using `std::function<void(const TilePlan&)>`. This **type erasure** hides the concrete callable behind one uniform runtime interface and can hold different copyable callable target types. The wrapper also carries lifetime and potential allocation/copy behavior of its stored target. The Phase 2 callback chapter already derived those details; here they become part of a choice about the planner's extensibility.

The lab constructs a runtime-erased lambda that accumulates the byte count from a 48-byte metadata plan and verifies the result. It does not benchmark `std::function` versus CRTP or claim one implementation allocated and the other did not. A callable stored inline by one standard library might allocate with another implementation or capture. A direct CRTP call could be inlined, but the binary may have more specializations and a larger instruction footprint. A virtual interface could be a better contract if plugins select recorder backends dynamically. `std::variant` could suit a closed list of recorder forms. The right choice depends on **when** the implementation can be selected and how long it must remain valid.

A practical comparative experiment would implement the same trace recorder using (a) a concrete direct call, (b) CRTP, (c) a virtual base interface and (d) `std::function`, with identical records and equivalent data dependencies. Measure the cost of generating and recording many plans, count allocations separately, inspect generated code, and check whether the compiler devirtualizes targets in whole-program builds. Then vary record size and callback capture size. Static dispatch has an optimization opportunity, not an unconditional performance certificate.

Policy-based design can itself introduce code bloat: every combination of format, policy and recorder may produce a distinct specialization. If dozens of variants change only a small setup decision, a runtime enum or erased callback at a coarse boundary may reduce binary size without harming the hot loop. This is why the final architecture should keep compile-time variation where it actually resolves work inside tight, frequently executed paths and avoid gratuitous template parameters elsewhere.

### The full walk through our example, including its failure boundaries

A request arrives to plan an 8-by-2 RGBA tile with a 64-byte application budget. The selected template format fixes the channel count at four. Runtime width and height produce sixteen pixels, and checked multiplication produces 64 bytes without overflow. The budget policy permits the request. The verified-payload tag requires the input span to contain exactly 64 bytes; the lab provides that payload. The `if constexpr` format branch marks the plan as alpha-bearing. The precomputed Morton table supplies the demonstration key 39 for coordinates (3,5). The CRTP recorder receives the completed `TilePlan`, increments its call count, and accumulates 64 bytes. Only after the recording call returns does the diagnostic global count increase. Under the tested single-threaded conditions, the plan has reached the recording boundary exactly once.

Now change one input at a time. An 8-by-3 RGBA plan would require 96 bytes, so the budget policy rejects it. The recorder count stays one. A 4-by-4 RGBA plan has a permitted 64-byte size but receives an empty input span under `VerifyPayloadTag`, so the buffer-length check rejects it. The recorder still reports one accepted plan. Next, a different planner specializes to three-channel RGB, uses the empty no-budget policy, and receives a 4-by-4 request under `MetadataOnlyTag`. Its 48-byte metadata plan is recorded without claiming the pixels were present. Finally, a deliberately enormous width triggers the multiplication overflow guard before any policy or recorder can accept a nonsensical byte count.

These observations exercise three separate validation layers: arithmetic safety, application budget, and readiness of the supplied byte span. They should not be collapsed into one vague "validated" flag. The manifest case is legal only because its semantics explicitly say it is **not** verified payload data. In a real renderer the next stages would also check format support, image extent limits, row pitch, actual device memory and queue readiness before presenting a GPU-usable texture. None of those native checks is implemented by this CPU laboratory.

There is also a transaction boundary worth examining. The `TilePlanner::prepare` function invokes `recorder_.record(plan)` and **then** increments a global counter. If a general recorder throws during `record`, the code has no guarantee that the recorder itself was unchanged before throwing. This is not Phase 2's full strong-guarantee asset publication transaction, and we must not reuse that label casually. For the included `CountingRecorder`, the operations used in the test are simple integer increments without ordinary throwing operations under the tested state, so the demonstrated failure paths all occur **before** the recorder call. A production recorder with disk, network or GPU side effects needs an explicit commit/rollback or compensation contract. Static dispatch does not supply atomicity.

### A mastery test that connects every mechanism without breaking the narrative

You should now be able to reconstruct the following reasoning without opening the source. Start with a mesh/texture asset pipeline and explain which values are known by the compiler and which appear only at runtime. Derive the source-byte size \(W\times H\times C\) and explain why multiplication must be checked before forming the result. Design a trait whose compile-time channel count is an `integral_constant` value, then show when `if constexpr` eliminates a format-specific branch and when `constexpr` merely permits a calculation to execute during compilation.

Build a legal compile-time tile configuration with `consteval`; explain why a runtime argument cannot replace a required constant expression. Derive a four-bit Morton table from interleaving coordinate bits and verify why (3,5) yields 39. Explain why the resulting `inline constexpr` table and the `constinit` mutable counter make different guarantees. Choose two budget policies and show why the checked arithmetic contract remains valid under both. Build a `RecorderBase<Derived>` and explain precisely why the derived cast is sound only for a genuinely appropriate object; contrast that static call with runtime type erasure. Use empty tag types to distinguish a verified payload from a metadata-only plan and explain why the latter cannot be used as proof of upload readiness. Finally draw the three policy holder layouts and state what EBO and `[[no_unique_address]]` *permit*—and what they do not promise across compilers.

The runnable witness for these exercises is [[tools/labs/compile_time_asset_policy_lab.cpp|the M014 tile-planning laboratory]]. It was compiled and executed with GCC 14.2 using C++20, `-Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined`; with Clang 17 under strict C++20 warnings; and with GCC using `-O2 -DNDEBUG`. All three executions reported `compile_time_asset_policy_lab: all checks passed`, and the executable also printed the three local policy-holder sizes. The committed lab's Git blob matches the locally tested file exactly (SHA `f4379dbce70c261f2832f8311a628e346f7471f7`). This is a tested teaching model, **not** a benchmark, Kairo/Maveb source audit, portable ABI certification, thread-safety proof, or graphics driver test.

Rebuild using the full source and command:

```sh
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  tools/labs/compile_time_asset_policy_lab.cpp -o /tmp/compile_time_asset_policy_lab
/tmp/compile_time_asset_policy_lab
```

This chapter provides an authored reconstruction of the exact original **M014, Parts 217–230**. It is inserted where our generic asset system requires a compile-time policy decision, rather than treating the fourteen numbered items as fourteen standalone lessons. [[Supplementary/Foundations#M014 — Parts 217–230: one tile planner and its compile-time policy decisions|The coverage map]] preserves every title; [[Supplementary/Sources and Code Anchors#Phase 4 additive reconstruction — M014 / Parts 217–230|the evidence ledger]] identifies executed and unverified claims; [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#The tile planner makes a choice — M014 reconstruction|the recall path]] reconnects the ideas to the same asset pipeline. No new original lesson transcript has been received, and the suggested `KairoMath/Matrix.cppm` / `KairoECS/SparseSet.cppm` project anchors have not been re-inspected in this phase.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/04 - Prove C++ Ownership with Runnable Experiments|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/06 - Grow Registries without Invalidating Identity|Next chapter]]
